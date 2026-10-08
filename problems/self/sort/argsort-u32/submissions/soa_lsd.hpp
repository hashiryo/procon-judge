#pragma once
// 値と添字を別々の u32 の配列に持ったまま、値の 11 bit + 11 bit + 10 bit の 3 回の LSD で一緒に振り分ける (pack_lsd の u64 に詰めない版)。
// LSD は安定なので、等しい値は元の順 (添字の小さい順) に残る。ヒストグラムは a を 1 回なめて作り、全要素が同じ値になる桁は飛ばす。
// 最初の振り分けは a を直接読んで添字 i と一緒に書き、最後の振り分けは添字だけを返り値の配列に書く。pack_lsd と比べると、1 要素を
// 振り分けるたびに書く先が値の配列と添字の配列の 2 か所になる。途中の作業用の配列 (u32 の 4 本) は huge page にする。256 個以下は
// 値と添字を u64 に詰めて std::sort に任せる。
#ifdef __linux__
#include <sys/mman.h>
#endif
#include <algorithm>
#include <cstdlib>
#include <memory>
#include <numeric>
#include <vector>
namespace argsort_soa_lsd {
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
inline std::vector<u32> argsort(const std::vector<u32>& a) {
 const size_t n= a.size();
 std::vector<u32> p(n);
 if(n <= 256) {
  std::vector<u64> b(n);
  for(size_t i= 0; i < n; ++i) b[i]= (u64(a[i]) << 32) | i;
  std::sort(b.begin(), b.end());
  for(size_t i= 0; i < n; ++i) p[i]= u32(b[i]);
  return p;
 }
 static constexpr int SH[3]= {0, 11, 22};
 static constexpr u32 SZ[3]= {2048, 2048, 1024};
 u32 cnt[3][2048]= {};
 const u32* key= a.data();
 for(size_t i= 0; i < n; ++i) {
  const u32 x= key[i];
  ++cnt[0][x & 2047], ++cnt[1][x >> 11 & 2047], ++cnt[2][x >> 22];
 }
 int use[3], k= 0;
 for(int d= 0; d < 3; ++d) {
  u32* c= cnt[d];
  bool trivial= false;
  for(u32 i= 0; i < SZ[d]; ++i)
   if(c[i]) {
    trivial= c[i] == n;
    break;
   }
  if(trivial) continue;
  u32 s= 0;
  for(u32 i= 0; i < SZ[d]; ++i) {
   u32 t= c[i];
   c[i]= s, s+= t;
  }
  use[k++]= d;
 }
 if(!k) {  // 全要素が同じ値
  std::iota(p.begin(), p.end(), 0u);
  return p;
 }
 if(k == 1) {  // 振り分け 1 回: a から添字を直接書く
  const int d= use[0], s= SH[d];
  const u32 m= SZ[d] - 1;
  u32* c= cnt[d];
  for(size_t i= 0; i < n; ++i) p[c[key[i] >> s & m]++]= u32(i);
  return p;
 }
 auto k1= alloc_huge<u32>(n), i1= alloc_huge<u32>(n);
 std::unique_ptr<u32, FreeDeleter> k2, i2;
 if(k >= 3) k2= alloc_huge<u32>(n), i2= alloc_huge<u32>(n);
 u32* ks[2]= {k1.get(), k2.get()};
 u32* is[2]= {i1.get(), i2.get()};
 {  // 最初の振り分け: a を読み、値と添字を書く
  const int d= use[0], s= SH[d];
  const u32 m= SZ[d] - 1;
  u32* c= cnt[d];
  u32 *dk= ks[0], *di= is[0];
  for(size_t i= 0; i < n; ++i) {
   const u32 x= key[i], pos= c[x >> s & m]++;
   dk[pos]= x, di[pos]= u32(i);
  }
 }
 u32 *sk= ks[0], *si= is[0];
 for(int j= 1; j < k - 1; ++j) {  // 途中の振り分け
  const int d= use[j], s= SH[d];
  const u32 m= SZ[d] - 1;
  u32* c= cnt[d];
  u32 *dk= ks[j & 1], *di= is[j & 1];
  for(size_t i= 0; i < n; ++i) {
   const u32 x= sk[i], pos= c[x >> s & m]++;
   dk[pos]= x, di[pos]= si[i];
  }
  sk= dk, si= di;
 }
 {  // 最後の振り分け: 添字だけを書く
  const int d= use[k - 1], s= SH[d];
  const u32 m= SZ[d] - 1;
  u32* c= cnt[d];
  for(size_t i= 0; i < n; ++i) p[c[sk[i] >> s & m]++]= si[i];
 }
 return p;
}
}  // namespace argsort_soa_lsd
inline std::vector<unsigned> run(const std::vector<unsigned>& a) { return argsort_soa_lsd::argsort(a); }
