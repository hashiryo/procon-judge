#pragma once
// lsd_adaptive の作業用の配列を 2 MB 境界で確保し、MADV_HUGEPAGE で huge page を頼んでから MADV_POPULATE_WRITE でまとめて用意させる版。
// 振り分けは 2048 か所に散らばって書くので、4 KB のページだと書き先が約 1000 ページにまたがり、TLB を外しやすい。huge page なら 2 ページで済む。
// THP が無効な kernel では MADV_HUGEPAGE は効かない。Linux 以外では何もしない。ほかは lsd_adaptive と同じ。
// LSD 基数ソートで、桁の切り方を入力から決める版。最初の走査で全要素の bit or と bit and を取り、要素によって
// 違う bit (or ^ and) の最下位から最上位までだけを、11 bit 以下の桁に等分して振り分ける。値が 30 bit に収まる
// 入力なら 10 bit × 3 回、下位 16 bit がすべて 0 なら 8 bit × 2 回になる。さらにヒストグラムを見て、全要素が
// 同じ値になる桁は飛ばす。振り分けの回数が奇数なら最初に a を作業用の配列へ写し、最後が a で終わるようにする。
// 回数の見込みが最初の走査で分かるので、写しはヒストグラムを作る走査でまとめて行う。作業用の配列は 0 で埋めない。
#ifdef __linux__
#include <sys/mman.h>
#endif
#include <algorithm>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <memory>
#include <vector>
namespace sort_lsd_adaptive_hp {
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
 u32* a= v.data();
 u32 o= 0, e= ~0u;
 for(size_t i= 0; i < n; ++i) o|= a[i], e&= a[i];
 const u32 diff= o ^ e;
 if(!diff) return;  // 全要素が同じ値
 const int lo= __builtin_ctz(diff), hi= 31 - __builtin_clz(diff);
 const int w= hi - lo + 1, p= (w + 10) / 11;
 int sh[3], bits[3];
 for(int d= 0, s= lo; d < p; ++d) {
  bits[d]= w / p + (d < w % p);
  sh[d]= s, s+= bits[d];
 }
 u32 cnt[3][2048];
 for(int d= 0; d < p; ++d) std::memset(cnt[d], 0, sizeof(u32) << bits[d]);
 auto buf= alloc_huge(n);
 u32* b= buf.get();
 const bool copied= p & 1;
 if(p == 1) {
  const u32 m0= (1u << bits[0]) - 1;
  for(size_t i= 0; i < n; ++i) {
   u32 x= a[i];
   b[i]= x;
   ++cnt[0][x >> sh[0] & m0];
  }
 } else if(p == 2) {
  const u32 m0= (1u << bits[0]) - 1, m1= (1u << bits[1]) - 1;
  for(size_t i= 0; i < n; ++i) {
   u32 x= a[i];
   ++cnt[0][x >> sh[0] & m0], ++cnt[1][x >> sh[1] & m1];
  }
 } else {
  const u32 m0= (1u << bits[0]) - 1, m1= (1u << bits[1]) - 1, m2= (1u << bits[2]) - 1;
  for(size_t i= 0; i < n; ++i) {
   u32 x= a[i];
   b[i]= x;
   ++cnt[0][x >> sh[0] & m0], ++cnt[1][x >> sh[1] & m1], ++cnt[2][x >> sh[2] & m2];
  }
 }
 // 全要素が同じ値になる桁を除き、残りの桁のヒストグラムを先頭からの位置に直す。
 int use[3], k= 0;
 for(int d= 0; d < p; ++d) {
  const u32 sz= 1u << bits[d];
  bool trivial= false;
  for(u32 i= 0; i < sz; ++i)
   if(cnt[d][i]) {
    trivial= cnt[d][i] == n;
    break;
   }
  if(trivial) continue;
  u32 s= 0;
  for(u32 i= 0; i < sz; ++i) {
   u32 t= cnt[d][i];
   cnt[d][i]= s, s+= t;
  }
  use[k++]= d;
 }
 // 奇数回なら写しから、偶数回なら a から始めると、最後が a で終わる。
 u32 *src= a, *dst= b;
 if(k & 1) {
  if(!copied) std::memcpy(b, a, n * sizeof(u32));
  src= b, dst= a;
 }
 for(int j= 0; j < k; ++j) {
  const int d= use[j], s= sh[d];
  const u32 m= (1u << bits[d]) - 1;
  u32* c= cnt[d];
  for(size_t i= 0; i < n; ++i) {
   u32 x= src[i];
   dst[c[x >> s & m]++]= x;
  }
  std::swap(src, dst);
 }
}
}  // namespace sort_lsd_adaptive_hp
inline void run(std::vector<unsigned>& a) { sort_lsd_adaptive_hp::sort(a); }
