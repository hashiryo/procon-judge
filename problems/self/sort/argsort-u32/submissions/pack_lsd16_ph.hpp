#pragma once
// pack_lsd16 (値の 16 bit × 2 回の LSD) の、返り値の配列も huge page にする版。返り値の vector は std::allocator で確保するしかないので、
// reserve で確保だけしてから、2 MB 境界に揃った中の部分に huge page を頼み、ページをまとめて用意させてから resize で 0 を埋める。
// 4 MB の返り値をそのまま 0 で埋めると、4 KB のページで 1000 回ほどページフォールトが起き、最後の振り分けも書き先が 65536 か所に
// 散らばる。その分がどれだけ効くかを pack_lsd16 と比べる。ほかは pack_lsd16 と同じ。
#ifdef __linux__
#include <sys/mman.h>
#endif
#include <algorithm>
#include <cstdint>
#include <cstdlib>
#include <memory>
#include <numeric>
#include <vector>
namespace argsort_pack_lsd16_ph {
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
// 長さ n の返り値の配列を作る。確保だけしてから、2 MB 境界に揃った中の部分に huge page を頼み、ページの境界に揃った中の部分を
// まとめて用意させてから 0 で埋める。
inline std::vector<u32> alloc_result(size_t n) {
 std::vector<u32> p;
 p.reserve(n);
#ifdef __linux__
 if(n) {
  constexpr uintptr_t H= uintptr_t(1) << 21, P= 4096;
  const uintptr_t s= reinterpret_cast<uintptr_t>(p.data()), e= s + n * sizeof(u32);
  const uintptr_t hs= (s + H - 1) & ~(H - 1), he= e & ~(H - 1);
  if(hs < he) madvise(reinterpret_cast<void*>(hs), he - hs, MADV_HUGEPAGE);
  const uintptr_t ps= (s + P - 1) & ~(P - 1), pe= e & ~(P - 1);
  if(ps < pe) madvise(reinterpret_cast<void*>(ps), pe - ps, 23);  // MADV_POPULATE_WRITE
 }
#endif
 p.resize(n);
 return p;
}
// pack_lsd と同じ 11 bit + 11 bit + 10 bit の 3 回の LSD (2^16 個未満で使う)。
inline void lsd11(const u32* key, size_t n, u32* p) {
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
 if(!k) {  // 全要素が同じ値
  std::iota(p, p + n, 0u);
  return;
 }
 if(k == 1) {  // 振り分け 1 回: a から添字を直接書く
  const int d= use[0], s= SH[d];
  const u32 m= SZ[d] - 1;
  u32* c= cnt[d];
  for(size_t i= 0; i < n; ++i) p[c[key[i] >> s & m]++]= u32(i);
  return;
 }
 auto b1= alloc_huge<u64>(n);
 std::unique_ptr<u64, FreeDeleter> b2;
 if(k >= 3) b2= alloc_huge<u64>(n);
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
 for(int j= 1; j < k - 1; ++j) {  // 途中の振り分け
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
 {  // 最後の振り分け: 添字だけを書く
  const int d= use[k - 1], s= SH[d] + 32;
  const u64 m= SZ[d] - 1;
  u32* c= cnt[d];
  for(size_t i= 0; i < n; ++i) {
   const u64 x= src[i];
   p[c[x >> s & m]++]= u32(x);
  }
 }
}
inline std::vector<u32> argsort(const std::vector<u32>& a) {
 const size_t n= a.size();
 std::vector<u32> p= alloc_result(n);
 if(n <= 256) {
  std::vector<u64> b(n);
  for(size_t i= 0; i < n; ++i) b[i]= (u64(a[i]) << 32) | i;
  std::sort(b.begin(), b.end());
  for(size_t i= 0; i < n; ++i) p[i]= u32(b[i]);
  return p;
 }
 const u32* key= a.data();
 if(n < (size_t(1) << 16)) {
  lsd11(key, n, p.data());
  return p;
 }
 std::vector<u32> cnt(2 << 16);
 u32 *c0= cnt.data(), *c1= c0 + (1 << 16);
 for(size_t i= 0; i < n; ++i) {
  const u32 x= key[i];
  ++c0[x & 0xFFFF], ++c1[x >> 16];
 }
 // 全要素が同じ値になる桁では、先頭の要素の桁の個数が n になる。
 const bool lo= c0[key[0] & 0xFFFF] != n, hi= c1[key[0] >> 16] != n;
 for(u32* c: {c0, c1}) {
  u32 s= 0;
  for(u32 i= 0; i < (1u << 16); ++i) {
   u32 t= c[i];
   c[i]= s, s+= t;
  }
 }
 if(!lo && !hi) {  // 全要素が同じ値
  std::iota(p.begin(), p.end(), 0u);
  return p;
 }
 if(!hi) {  // 振り分け 1 回: a から添字を直接書く
  for(size_t i= 0; i < n; ++i) p[c0[key[i] & 0xFFFF]++]= u32(i);
  return p;
 }
 if(!lo) {
  for(size_t i= 0; i < n; ++i) p[c1[key[i] >> 16]++]= u32(i);
  return p;
 }
 auto bb= alloc_huge<u64>(n);
 u64* b= bb.get();
 for(size_t i= 0; i < n; ++i) {  // 下の 16 bit で振り分け、その場で詰める
  const u32 x= key[i];
  b[c0[x & 0xFFFF]++]= (u64(x) << 32) | i;
 }
 for(size_t i= 0; i < n; ++i) {  // 上の 16 bit で振り分け、添字だけを書く
  const u64 x= b[i];
  p[c1[x >> 48]++]= u32(x);
 }
 return p;
}
}  // namespace argsort_pack_lsd16_ph
inline std::vector<unsigned> run(const std::vector<unsigned>& a) { return argsort_pack_lsd16_ph::argsort(a); }
