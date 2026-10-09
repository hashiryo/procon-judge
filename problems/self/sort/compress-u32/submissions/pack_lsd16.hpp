#pragma once
// 値を上位 32 bit、添字を下位 32 bit に詰めた u64 を、self-sort-argsort-u32 の pack_lsd16 と同じ値の 16 bit × 2 回の LSD で並べ、
// 並べた列を先頭から走査する。値が変わるたびに順位を 1 増やして xs に足し、添字の位置に順位を書く。argsort では最後の振り分けで
// 添字だけを書いたが、ここでは走査で値を比べるので、最後の振り分けも詰めた u64 を書く。ヒストグラムは a を 1 回なめて 2 つ作り、
// 全要素が同じ値になる桁は飛ばす。最初の振り分けは a を直接読んでその場で詰める。作業用の配列 (u64、8 MB を 2 本) は huge page に
// する。2^16 個未満は argsort の pack_lsd と同じ 11 bit × 3 回、256 個以下は詰めて std::sort に任せる。
#ifdef __linux__
#include <sys/mman.h>
#endif
#include <algorithm>
#include <cstdlib>
#include <memory>
#include <vector>
namespace compress_pack_lsd16 {
using u32= unsigned;
using u64= unsigned long long;
struct FreeDeleter {
 void operator()(void* p) const { std::free(p); }
};
// 2 MB 境界で確保し、huge page を頼んでから、触る前にまとめて用意させる。
template <class T> inline std::unique_ptr<T, FreeDeleter> alloc_huge(size_t n) {
 constexpr size_t H= size_t(1) << 21;
 const size_t bytes= (n * sizeof(T) + H - 1) & ~(H - 1);
 void* p= std::aligned_alloc(H, bytes);
#ifdef __linux__
 madvise(p, bytes, MADV_HUGEPAGE);
 madvise(p, bytes, 23);  // MADV_POPULATE_WRITE
#endif
 return std::unique_ptr<T, FreeDeleter>(static_cast<T*>(p));
}
// 値の順に並んだ詰めた u64 の列 s[0, n) (n >= 1) を先頭から走査する。値が変わるたびに順位を 1 増やして xs に足し、a の添字の
// 位置に順位を書く。
inline std::vector<u32> scan(const u64* s, size_t n, u32* a) {
 std::vector<u32> xs;
 xs.reserve(n);
 u32 prev= u32(s[0] >> 32), r= 0;
 xs.push_back(prev);
 for(size_t i= 0; i < n; ++i) {
  const u64 x= s[i];
  const u32 v= u32(x >> 32);
  if(v != prev) xs.push_back(v), prev= v, ++r;
  a[u32(x)]= r;
 }
 return xs;
}
// 全要素が同じ値 (n >= 1)。
inline std::vector<u32> all_same(u32* a, size_t n) {
 std::vector<u32> xs(1, a[0]);
 std::fill(a, a + n, 0u);
 return xs;
}
// argsort の pack_lsd と同じ 11 bit + 11 bit + 10 bit の 3 回の LSD で詰めた u64 を並べ、走査する (2^16 個未満で使う)。
inline std::vector<u32> lsd11(u32* key, size_t n) {
 static constexpr int SH[3]= {0, 11, 22};
 static constexpr u32 SZ[3]= {2048, 2048, 1024};
 u32 cnt[3][2048]= {};
 for(size_t i= 0; i < n; ++i) {
  const u32 x= key[i];
  ++cnt[0][x & 2047], ++cnt[1][x >> 11 & 2047], ++cnt[2][x >> 22];
 }
 int use[3], k= 0;
 for(int d= 0; d < 3; ++d) {
  u32* c= cnt[d];
  if(c[key[0] >> SH[d] & (SZ[d] - 1)] == n) continue;  // 全要素が同じ値になる桁
  u32 s= 0;
  for(u32 i= 0; i < SZ[d]; ++i) {
   u32 t= c[i];
   c[i]= s, s+= t;
  }
  use[k++]= d;
 }
 if(!k) return all_same(key, n);
 auto b1= alloc_huge<u64>(n);
 std::unique_ptr<u64, FreeDeleter> b2;
 if(k >= 2) b2= alloc_huge<u64>(n);
 u64* bufs[2]= {b1.get(), b2.get()};
 {  // 最初の振り分け: a を読んでその場で詰める
  const int d= use[0], s= SH[d];
  const u32 m= SZ[d] - 1;
  u32* c= cnt[d];
  u64* dst= bufs[0];
  for(size_t i= 0; i < n; ++i) {
   const u32 x= key[i];
   dst[c[x >> s & m]++]= (u64(x) << 32) | i;
  }
 }
 u64* src= bufs[0];
 for(int j= 1; j < k; ++j) {
  const int d= use[j], s= SH[d] + 32;
  const u64 m= SZ[d] - 1;
  u32* c= cnt[d];
  u64* dst= bufs[j & 1];
  for(size_t i= 0; i < n; ++i) {
   const u64 x= src[i];
   dst[c[x >> s & m]++]= x;
  }
  src= dst;
 }
 return scan(src, n, key);
}
inline std::vector<u32> compress(std::vector<u32>& a) {
 const size_t n= a.size();
 u32* key= a.data();
 if(!n) return {};
 if(n <= 256) {
  u64 b[256]= {};
  for(size_t i= 0; i < n; ++i) b[i]= (u64(key[i]) << 32) | i;
  std::sort(b, b + n);
  return scan(b, n, key);
 }
 if(n < (size_t(1) << 16)) return lsd11(key, n);
 std::vector<u32> cnt(2 << 16);
 u32 *c0= cnt.data(), *c1= c0 + (1 << 16);
 for(size_t i= 0; i < n; ++i) {
  const u32 x= key[i];
  ++c0[x & 0xFFFF], ++c1[x >> 16];
 }
 // 全要素が同じ値になる桁では、先頭の要素の桁の個数が n になる。
 const bool lo= c0[key[0] & 0xFFFF] != n, hi= c1[key[0] >> 16] != n;
 if(!lo && !hi) return all_same(key, n);
 for(u32* c: {c0, c1}) {
  u32 s= 0;
  for(u32 i= 0; i < (1u << 16); ++i) {
   u32 t= c[i];
   c[i]= s, s+= t;
  }
 }
 auto bb= alloc_huge<u64>(n);
 u64* b= bb.get();
 if(!lo || !hi) {  // 振り分け 1 回で並ぶ
  const int s= lo ? 0 : 16;
  u32* c= lo ? c0 : c1;
  for(size_t i= 0; i < n; ++i) {
   const u32 x= key[i];
   b[c[x >> s & 0xFFFF]++]= (u64(x) << 32) | i;
  }
  return scan(b, n, key);
 }
 auto db= alloc_huge<u64>(n);
 u64* d= db.get();
 for(size_t i= 0; i < n; ++i) {  // 下の 16 bit で振り分け、その場で詰める
  const u32 x= key[i];
  b[c0[x & 0xFFFF]++]= (u64(x) << 32) | i;
 }
 for(size_t i= 0; i < n; ++i) {  // 上の 16 bit で振り分ける
  const u64 x= b[i];
  d[c1[x >> 48]++]= x;
 }
 return scan(d, n, key);
}
}  // namespace compress_pack_lsd16
inline std::vector<unsigned> run(std::vector<unsigned>& a) { return compress_pack_lsd16::compress(a); }
