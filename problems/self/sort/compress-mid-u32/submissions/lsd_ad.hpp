#pragma once
// 値を上位 32 bit、添字を下位 32 bit に詰めた u64 を、要素によって違う bit だけを桁に分けた LSD で並べ、並べた列を先頭から走査する
// (走査は pack_lsd16 と同じ)。値の bit or と bit and から要素によって違う bit の範囲 [lo, hi] を求め、その幅 w を 11 bit 以下の
// ほぼ等しい幅の桁 ceil(w / 11) 個に分ける (10^9 までの値なら 10 bit × 3 回、16 bit だけ違うなら 8 bit × 2 回)。ヒストグラムは
// a を 1 回なめて全部の桁の分をまとめて作り、最初の振り分けは a を直接読んでその場で詰める。全要素が同じ値なら振り分けない。
// 作業用の配列 (u64 を 2 本) と返す xs は、2^19 byte 以上なら Linux では 2 MB 境界の mmap に置いて huge page を頼む (NeoLibrary の
// neo/sort/compress.hpp の Buf と alloc_xs と同じ取り方)。aoj-2674 で、10^5 個の値なら AMD の 7763 では 11 bit ずつ 3 回の LSD が
// compress (塊をネットワークで並べる形) より速かったので、長さごとに比べるために置く。
#ifdef __linux__
#include <sys/mman.h>
#endif
#include <algorithm>
#include <cstdint>
#include <new>
#include <vector>
namespace compress_lsd_ad {
using u32= unsigned;
using u64= unsigned long long;
struct Buf {
 u64* p;
 size_t mapped= 0;
 explicit Buf(size_t n) {
#ifdef __linux__
  constexpr size_t H= size_t(1) << 21;
  if(n * sizeof(u64) >= (size_t(1) << 19)) {
   const size_t bytes= (n * sizeof(u64) + H - 1) & ~(H - 1);
   void* m= mmap(nullptr, bytes + H, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
   if(m != MAP_FAILED) {
    char *q= static_cast<char*>(m), *a= reinterpret_cast<char*>((reinterpret_cast<uintptr_t>(q) + H - 1) & ~(H - 1));
    if(a != q) munmap(q, a - q);
    munmap(a + bytes, q + H - a);
    madvise(a, bytes, MADV_HUGEPAGE);
    madvise(a, bytes, 23);  // MADV_POPULATE_WRITE
    p= reinterpret_cast<u64*>(a), mapped= bytes;
    return;
   }
  }
#endif
  p= static_cast<u64*>(::operator new(n * sizeof(u64)));
 }
 ~Buf() {
#ifdef __linux__
  if(mapped) {
   munmap(p, mapped);
   return;
  }
#endif
  ::operator delete(p);
 }
 Buf(const Buf&)= delete;
 Buf& operator=(const Buf&)= delete;
};
inline std::vector<u32> alloc_xs(size_t n) {
 std::vector<u32> xs;
 xs.reserve(n);
#ifdef __linux__
 if(n * sizeof(u32) >= (size_t(1) << 19)) {
  constexpr uintptr_t H= uintptr_t(1) << 21, P= 4096;
  const uintptr_t s= reinterpret_cast<uintptr_t>(xs.data()), e= s + n * sizeof(u32);
  const uintptr_t hs= (s + H - 1) & ~(H - 1), he= e & ~(H - 1);
  if(hs < he) madvise(reinterpret_cast<void*>(hs), he - hs, MADV_HUGEPAGE);
  const uintptr_t ps= (s + P - 1) & ~(P - 1), pe= e & ~(P - 1);
  if(ps < pe) madvise(reinterpret_cast<void*>(ps), pe - ps, 23);  // MADV_POPULATE_WRITE
 }
#endif
 return xs;
}
inline std::vector<u32> compress(std::vector<u32>& a) {
 const size_t n= a.size();
 if(!n) return {};
 u32 o= 0, e= ~0u;
 for(u32 x: a) o|= x, e&= x;
 const u32 diff= o ^ e;
 if(!diff) {
  std::vector<u32> xs(1, a[0]);
  std::fill(a.begin(), a.end(), 0u);
  return xs;
 }
 const int hi= 31 - __builtin_clz(diff), lo= __builtin_ctz(diff), w= hi - lo + 1, P= (w + 10) / 11, d= (w + P - 1) / P;
 int sh[3], wd[3];
 for(int p= 0; p < P; ++p) sh[p]= lo + p * d, wd[p]= std::min(d, w - p * d);
 std::vector<u32> cnt(size_t(P) << d, 0);
 u32* c0= cnt.data();
 if(P == 1) {
  const u32 m0= (1u << wd[0]) - 1;
  for(u32 x: a) ++c0[x >> sh[0] & m0];
 } else if(P == 2) {
  u32* c1= c0 + (size_t(1) << d);
  const u32 m0= (1u << wd[0]) - 1, m1= (1u << wd[1]) - 1;
  for(u32 x: a) ++c0[x >> sh[0] & m0], ++c1[x >> sh[1] & m1];
 } else {
  u32 *c1= c0 + (size_t(1) << d), *c2= c1 + (size_t(1) << d);
  const u32 m0= (1u << wd[0]) - 1, m1= (1u << wd[1]) - 1, m2= (1u << wd[2]) - 1;
  for(u32 x: a) ++c0[x >> sh[0] & m0], ++c1[x >> sh[1] & m1], ++c2[x >> sh[2] & m2];
 }
 for(int p= 0; p < P; ++p) {
  u32* c= c0 + (size_t(p) << d);
  for(u32 i= 0, s= 0; i < (1u << wd[p]); ++i) {
   const u32 t= c[i];
   c[i]= s, s+= t;
  }
 }
 Buf b0(n), b1(n);
 u64 *src= b0.p, *dst= b1.p;
 {
  const u32 m0= (1u << wd[0]) - 1;
  for(size_t i= 0; i < n; ++i) {
   const u32 x= a[i];
   src[c0[x >> sh[0] & m0]++]= u64(x) << 32 | i;
  }
 }
 for(int p= 1; p < P; ++p) {
  u32* c= c0 + (size_t(p) << d);
  const int s= 32 + sh[p];
  const u64 m= (u64(1) << wd[p]) - 1;
  for(size_t i= 0; i < n; ++i) {
   const u64 k= src[i];
   dst[c[k >> s & m]++]= k;
  }
  std::swap(src, dst);
 }
 std::vector<u32> xs= alloc_xs(n);
 u32 prev= u32(src[0] >> 32), r= 0;
 xs.push_back(prev);
 for(size_t i= 0; i < n; ++i) {
  const u64 k= src[i];
  const u32 v= u32(k >> 32);
  if(v != prev) xs.push_back(v), prev= v, ++r;
  a[u32(k)]= r;
 }
 return xs;
}
}  // namespace compress_lsd_ad
inline std::vector<unsigned> run(std::vector<unsigned>& a) { return compress_lsd_ad::compress(a); }
