#pragma once
// lsd11 (11 bit + 11 bit + 10 bit の 3 回) の作業用の配列を lsd_adaptive_hp と同じく huge page にし、ヒストグラムを見て全要素が
// 同じ値になる桁を飛ばす版。lsd_adaptive_hp との違いは、最初の bit or と bit and の走査が無く、走査がヒストグラムを作る 1 回で済む
// こと。桁の切り方は固定なので、値が 30 bit に収まる入力でも 3 回 (最上位の桁が全要素で同じなら 2 回) になる。
// ヒストグラムを作る走査で a を作業用の配列へ写しておき、残る回数が奇数なら写しから、偶数なら a から振り分けて a で終える。
#ifdef __linux__
#include <sys/mman.h>
#endif
#include <algorithm>
#include <cstdlib>
#include <memory>
#include <vector>
namespace sort_lsd11_hp {
using u32= unsigned;
struct FreeDeleter {
 void operator()(void* p) const { std::free(p); }
};
// 2 MB 境界で確保し、huge page を頼んでから、触る前にまとめて用意させる。
inline std::unique_ptr<u32, FreeDeleter> alloc_huge(size_t n) {
 constexpr size_t H= size_t(1) << 21;
 const size_t bytes= (n * sizeof(u32) + H - 1) & ~(H - 1);
 void* p= std::aligned_alloc(H, bytes);
#ifdef __linux__
 madvise(p, bytes, MADV_HUGEPAGE);
 madvise(p, bytes, 23);  // MADV_POPULATE_WRITE
#endif
 return std::unique_ptr<u32, FreeDeleter>(static_cast<u32*>(p));
}
inline void sort(std::vector<u32>& v) {
 const size_t n= v.size();
 if(n <= 256) return std::sort(v.begin(), v.end());
 static constexpr int SH[3]= {0, 11, 22};
 static constexpr u32 MK[3]= {2047, 2047, 1023};
 u32 c0[2048]= {}, c1[2048]= {}, c2[1024]= {};
 auto buf= alloc_huge(n);
 u32 *a= v.data(), *b= buf.get();
 for(size_t i= 0; i < n; ++i) {
  u32 x= a[i];
  b[i]= x;
  ++c0[x & 2047], ++c1[x >> 11 & 2047], ++c2[x >> 22];
 }
 u32* cs[3]= {c0, c1, c2};
 int use[3], k= 0;
 for(int d= 0; d < 3; ++d) {
  u32* c= cs[d];
  bool trivial= false;
  for(u32 i= 0; i <= MK[d]; ++i)
   if(c[i]) {
    trivial= c[i] == n;
    break;
   }
  if(trivial) continue;
  u32 s= 0;
  for(u32 i= 0; i <= MK[d]; ++i) {
   u32 t= c[i];
   c[i]= s, s+= t;
  }
  use[k++]= d;
 }
 u32 *src= a, *dst= b;
 if(k & 1) src= b, dst= a;
 for(int j= 0; j < k; ++j) {
  const int d= use[j], sh= SH[d];
  const u32 mk= MK[d];
  u32* c= cs[d];
  for(size_t i= 0; i < n; ++i) {
   u32 x= src[i];
   dst[c[x >> sh & mk]++]= x;
  }
  std::swap(src, dst);
 }
}
}  // namespace sort_lsd11_hp
inline void run(std::vector<unsigned>& a) { sort_lsd11_hp::sort(a); }
