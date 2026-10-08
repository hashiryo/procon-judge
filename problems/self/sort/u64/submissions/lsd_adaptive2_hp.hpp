#pragma once
// lsd_adaptive_hp の最初の走査で、鍵の bit or と bit and に加えて最小値と最大値も取り、「要素によって違う bit の幅」と「最大値と最小値の
// 差の幅」の狭い方で桁を切る版。差の幅を使うときは鍵から最小値を引いてから桁を取り出す。値が一部に固まっていて上の bit まで変わる
// 入力 (i64 で 0 をまたぐ [-10^9, 10^9] など) で、振り分けの回数が減る。下位の bit がすべて 0 の入力は、これまでどおり bit の幅で切る。
// self-sort-u32 の lsd_adaptive_hp を 64 bit にした版。最初の走査で全要素の鍵の bit or と bit and を取り、要素によって違う bit の
// 最下位から最上位までを 11 bit 以下の桁に等分して、下の桁から振り分ける。64 bit 全域なら 6 回、値が 10^18 以下なら 60 bit で
// 6 回、10^9 以下なら 30 bit で 3 回になる。ヒストグラムを見て全要素が同じ値になる桁は飛ばす。振り分けの回数の見込みが奇数なら、
// ヒストグラムを作る走査で a を作業用の配列へ写し、写しから振り分けて a で終える。作業用の配列は 2 MB 境界で確保し、huge page を
// 頼んでから MADV_POPULATE_WRITE でまとめて用意させる (u32 では 1 割から 2 割効いた)。鍵は値そのもの。
#ifdef __linux__
#include <sys/mman.h>
#endif
#include <algorithm>
#include <cstdlib>
#include <cstring>
#include <memory>
#include <vector>
namespace sort_lsd_adaptive2_hp {
using T= unsigned long long;
using u64= unsigned long long;
using u32= unsigned;
// 桁を取り出すときの鍵。鍵の符号なしの順が値の順になるようにする。
inline u64 key(T x) { return x; }
struct FreeDeleter {
 void operator()(void* p) const { std::free(p); }
};
// 2 MB 境界で確保し、huge page を頼んでから、触る前にまとめて用意させる。
inline std::unique_ptr<T, FreeDeleter> alloc_huge(size_t n) {
 constexpr size_t H= size_t(1) << 21;
 const size_t bytes= (n * sizeof(T) + H - 1) & ~(H - 1);
 void* p= std::aligned_alloc(H, bytes);
#ifdef __linux__
 madvise(p, bytes, MADV_HUGEPAGE);
 madvise(p, bytes, 23);  // MADV_POPULATE_WRITE
#endif
 return std::unique_ptr<T, FreeDeleter>(static_cast<T*>(p));
}
// P 個の桁のヒストグラムを 1 回の走査で作る。COPY なら a を b へ写す。
template <int P, bool COPY> inline void histogram(const T* a, T* b, size_t n, u64 base, const int* sh, const u64* mk, u32 (*cnt)[2048]) {
 for(size_t i= 0; i < n; ++i) {
  const T x= a[i];
  if constexpr(COPY) b[i]= x;
  const u64 k= key(x) - base;
  for(int d= 0; d < P; ++d) ++cnt[d][k >> sh[d] & mk[d]];
 }
}
template <bool COPY> inline void histogram(int p, const T* a, T* b, size_t n, u64 base, const int* sh, const u64* mk, u32 (*cnt)[2048]) {
 switch(p) {
  case 1: return histogram<1, COPY>(a, b, n, base, sh, mk, cnt);
  case 2: return histogram<2, COPY>(a, b, n, base, sh, mk, cnt);
  case 3: return histogram<3, COPY>(a, b, n, base, sh, mk, cnt);
  case 4: return histogram<4, COPY>(a, b, n, base, sh, mk, cnt);
  case 5: return histogram<5, COPY>(a, b, n, base, sh, mk, cnt);
  default: return histogram<6, COPY>(a, b, n, base, sh, mk, cnt);
 }
}
inline void sort(std::vector<T>& v) {
 const size_t n= v.size();
 if(n <= 256) return std::sort(v.begin(), v.end());
 T* a= v.data();
 u64 o= 0, e= ~0ull, mn= ~0ull, mx= 0;
 for(size_t i= 0; i < n; ++i) {
  const u64 k= key(a[i]);
  o|= k, e&= k, mn= std::min(mn, k), mx= std::max(mx, k);
 }
 const u64 diff= o ^ e;
 if(!diff) return;  // 全要素が同じ値
 // 鍵から base を引いた値の bit [lo, lo + w) で振り分ける。要素によって違う bit の幅と、最大値と最小値の差の幅の狭い方を使う。
 int lo= __builtin_ctzll(diff), w= 64 - __builtin_clzll(diff) - lo;
 u64 base= 0;
 if(const int wm= 64 - __builtin_clzll(mx - mn); wm < w) lo= 0, w= wm, base= mn;
 const int p= (w + 10) / 11;
 int sh[6]= {}, bits[6]= {};
 u64 mk[6]= {};
 for(int d= 0, s= lo; d < p; ++d) {
  bits[d]= w / p + (d < w % p);
  sh[d]= s, s+= bits[d];
  mk[d]= (u64(1) << bits[d]) - 1;
 }
 u32 cnt[6][2048];
 for(int d= 0; d < p; ++d) std::memset(cnt[d], 0, sizeof(u32) << bits[d]);
 auto buf= alloc_huge(n);
 T* b= buf.get();
 const bool copied= p & 1;
 if(copied) histogram<true>(p, a, b, n, base, sh, mk, cnt);
 else histogram<false>(p, a, b, n, base, sh, mk, cnt);
 // 全要素が同じ値になる桁を除き、残りの桁のヒストグラムを先頭からの位置に直す。
 int use[6], k= 0;
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
 T *src= a, *dst= b;
 if(k & 1) {
  if(!copied) std::memcpy(b, a, n * sizeof(T));
  src= b, dst= a;
 }
 for(int j= 0; j < k; ++j) {
  const int d= use[j], s= sh[d];
  const u64 m= mk[d];
  u32* c= cnt[d];
  for(size_t i= 0; i < n; ++i) {
   const T x= src[i];
   dst[c[(key(x) - base) >> s & m]++]= x;
  }
  std::swap(src, dst);
 }
}
}  // namespace sort_lsd_adaptive2_hp
inline void run(std::vector<unsigned long long>& a) { sort_lsd_adaptive2_hp::sort(a); }
