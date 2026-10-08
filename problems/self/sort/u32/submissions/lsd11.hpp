#pragma once
// LSD 基数ソート、11 bit + 11 bit + 10 bit の 3 回。ヒストグラムは 2048 + 2048 + 1024 個で、合わせて 20 KB と L1 に載る。
// 3 回 (奇数回) だと a から振り分け始めると作業用の配列で終わるので、ヒストグラムを作る走査で a を作業用の配列へ
// 写しておき、作業用の配列 → a → 作業用の配列 → a と振り分ける。作業用の配列は 0 で埋めない。256 個以下は std::sort。
#include <algorithm>
#include <memory>
#include <vector>
namespace sort_lsd11 {
using u32= unsigned;
inline void sort(std::vector<u32>& v) {
 const size_t n= v.size();
 if(n <= 256) return std::sort(v.begin(), v.end());
 static constexpr int SH[3]= {0, 11, 22};
 static constexpr u32 MK[3]= {2047, 2047, 1023};
 u32 c0[2048]= {}, c1[2048]= {}, c2[1024]= {};
 std::unique_ptr<u32[]> buf(new u32[n]);
 u32 *a= v.data(), *b= buf.get();
 for(size_t i= 0; i < n; ++i) {
  u32 x= a[i];
  b[i]= x;
  ++c0[x & 2047], ++c1[x >> 11 & 2047], ++c2[x >> 22];
 }
 u32* cs[3]= {c0, c1, c2};
 for(int d= 0; d < 3; ++d) {
  u32 s= 0;
  for(u32 i= 0; i <= MK[d]; ++i) {
   u32 t= cs[d][i];
   cs[d][i]= s, s+= t;
  }
 }
 u32 *src= b, *dst= a;
 for(int d= 0; d < 3; ++d) {
  u32* c= cs[d];
  const int sh= SH[d];
  const u32 mk= MK[d];
  for(size_t i= 0; i < n; ++i) {
   u32 x= src[i];
   dst[c[x >> sh & mk]++]= x;
  }
  std::swap(src, dst);
 }
}
}  // namespace sort_lsd11
inline void run(std::vector<unsigned>& a) { sort_lsd11::sort(a); }
