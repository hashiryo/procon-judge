#pragma once
// AVX2 で分割するクイックソート。8 個ずつ読んで軸と比べ、movemask の 8 bit で表を引いて、軸以下の要素が前、
// 軸より大きい要素が後ろに来るように vpermd で並べ替え、同じベクタを左の書き込み位置と右の書き込み位置の両方に
// 書く (vxsort や Bramas の AVX-512 版と同じ形)。その場で分けるため、最初に両端の 8 個ずつをレジスタに取って
// 空きを作り、空きの少ない側から読む。比べるのは符号 bit を反転した値どうしの符号付き比較 (AVX2 に符号なしの
// 比較が無いため)。軸は 3 点か 9 点の中央値。全要素が軸以下になったときは軸が最大値なので、軸と等しい要素を
// 右に分けて左だけを続ける (重複の多い入力で止まらないため)。32 個以下の塊は、最大値で埋めて 8 / 16 / 32 個の
// bitonic のソーティングネットワークで並べる。再帰が深くなりすぎたら std::sort に任せる。
#ifdef USE_SIMDE
#include <simde/x86/avx2.h>
#else
#include <immintrin.h>
#endif
#include <algorithm>
#include <cstring>
#include <vector>
namespace sort_qsort_avx2 {
using u32= unsigned;
// 表の m 番目: bit i が立っている lane (軸より大きい) を後ろに、立っていない lane を前に集める vpermd の添字を
// 3 bit ずつ詰めたもの。
struct PermTable {
 u32 t[256];
 constexpr PermTable(): t() {
  for(int m= 0; m < 256; ++m) {
   u32 packed= 0;
   int k= 0;
   for(int i= 0; i < 8; ++i)
    if(!(m >> i & 1)) packed|= u32(i) << (3 * k++);
   for(int i= 0; i < 8; ++i)
    if(m >> i & 1) packed|= u32(i) << (3 * k++);
   t[m]= packed;
  }
 }
};
inline constexpr PermTable PERM{};
inline void part8(__m256i v, __m256i sign, __m256i pv, __m256i shifts, u32*& wl, u32*& wr) {
 __m256i gt= _mm256_cmpgt_epi32(_mm256_xor_si256(v, sign), pv);
 int m= _mm256_movemask_ps(_mm256_castsi256_ps(gt));
 __m256i idx= _mm256_srlv_epi32(_mm256_set1_epi32(int(PERM.t[m])), shifts);
 __m256i w= _mm256_permutevar8x32_epi32(v, idx);
 int nr= __builtin_popcount(m);
 _mm256_storeu_si256((__m256i*)wl, w);
 _mm256_storeu_si256((__m256i*)(wr - 8), w);
 wl+= 8 - nr, wr-= nr;
}
// [a, a + n) を、p 以下を左、p より大きいものを右に分け、左の個数を返す。n >= 16 が前提。
inline size_t partition(u32* a, size_t n, u32 p) {
 const __m256i sign= _mm256_set1_epi32(int(0x80000000u));
 const __m256i pv= _mm256_set1_epi32(int(p ^ 0x80000000u));
 const __m256i shifts= _mm256_setr_epi32(0, 3, 6, 9, 12, 15, 18, 21);
 const __m256i vl= _mm256_loadu_si256((const __m256i*)a), vr= _mm256_loadu_si256((const __m256i*)(a + n - 8));
 // [wl, rl) と [rr, wr) が空き。読む前の空きは合わせて 16 個で、少ない側から読めば両側とも 8 個以上空く。
 u32 *rl= a + 8, *rr= a + n - 8, *wl= a, *wr= a + n;
 while(rr - rl >= 8) {
  __m256i v;
  if(rl - wl <= wr - rr) v= _mm256_loadu_si256((const __m256i*)rl), rl+= 8;
  else rr-= 8, v= _mm256_loadu_si256((const __m256i*)rr);
  part8(v, sign, pv, shifts, wl, wr);
 }
 // 残りの 8 個未満は、写してから 1 個ずつ置く (読んでいない要素を上書きしないため)。
 u32 tail[8];
 const size_t t= rr - rl;
 std::memcpy(tail, rl, t * sizeof(u32));
 for(size_t i= 0; i < t; ++i) {
  if(tail[i] <= p) *wl++= tail[i];
  else *--wr= tail[i];
 }
 part8(vl, sign, pv, shifts, wl, wr);
 part8(vr, sign, pv, shifts, wl, wr);
 return wl - a;
}
// bitonic のソーティングネットワーク。stepD<MASK> は距離 D の lane どうしを比べ、MASK の bit が立つ lane に大きい方を置く。
inline __m256i vmin(__m256i a, __m256i b) { return _mm256_min_epu32(a, b); }
inline __m256i vmax(__m256i a, __m256i b) { return _mm256_max_epu32(a, b); }
template <int MASK> inline __m256i step1(__m256i v) {
 __m256i q= _mm256_shuffle_epi32(v, 0xB1);
 return _mm256_blend_epi32(vmin(v, q), vmax(v, q), MASK);
}
template <int MASK> inline __m256i step2(__m256i v) {
 __m256i q= _mm256_shuffle_epi32(v, 0x4E);
 return _mm256_blend_epi32(vmin(v, q), vmax(v, q), MASK);
}
template <int MASK> inline __m256i step4(__m256i v) {
 __m256i q= _mm256_permute4x64_epi64(v, 0x4E);
 return _mm256_blend_epi32(vmin(v, q), vmax(v, q), MASK);
}
// bitonic な 8 個を昇順にする。
inline __m256i merge8(__m256i v) { return step1<0xAA>(step2<0xCC>(step4<0xF0>(v))); }
// 任意の 8 個を昇順にする。2 個ずつ昇順と降順、4 個ずつ昇順と降順に並べて bitonic にしてから merge8。
inline __m256i sort8(__m256i v) { return merge8(step1<0x5A>(step2<0x3C>(step1<0x66>(v)))); }
inline __m256i reverse8(__m256i v) { return _mm256_permutevar8x32_epi32(v, _mm256_setr_epi32(7, 6, 5, 4, 3, 2, 1, 0)); }
// a, b を並べた 16 個が bitonic なら昇順にする。
inline void merge16(__m256i& a, __m256i& b) {
 __m256i l= vmin(a, b), h= vmax(a, b);
 a= merge8(l), b= merge8(h);
}
inline void sort16(__m256i& a, __m256i& b) {
 a= sort8(a), b= reverse8(sort8(b));
 merge16(a, b);
}
inline void sort32(__m256i& a, __m256i& b, __m256i& c, __m256i& d) {
 sort16(a, b), sort16(c, d);
 // 後ろの 16 個を逆順にして (reverse8(d), reverse8(c)) とつなぐと 32 個が bitonic になる。
 __m256i rc= reverse8(d), rd= reverse8(c);
 __m256i l0= vmin(a, rc), h0= vmax(a, rc), l1= vmin(b, rd), h1= vmax(b, rd);
 merge16(l0, l1), merge16(h0, h1);
 a= l0, b= l1, c= h0, d= h1;
}
inline void small_sort(u32* a, size_t n) {
 if(n <= 1) return;
 alignas(32) u32 buf[32];
 const size_t blk= n <= 8 ? 8 : n <= 16 ? 16 : 32;
 std::memcpy(buf, a, n * sizeof(u32));
 for(size_t i= n; i < blk; ++i) buf[i]= ~0u;
 __m256i* q= (__m256i*)buf;
 if(blk == 8) {
  _mm256_store_si256(q, sort8(_mm256_load_si256(q)));
 } else if(blk == 16) {
  __m256i x= _mm256_load_si256(q), y= _mm256_load_si256(q + 1);
  sort16(x, y);
  _mm256_store_si256(q, x), _mm256_store_si256(q + 1, y);
 } else {
  __m256i x= _mm256_load_si256(q), y= _mm256_load_si256(q + 1), z= _mm256_load_si256(q + 2), w= _mm256_load_si256(q + 3);
  sort32(x, y, z, w);
  _mm256_store_si256(q, x), _mm256_store_si256(q + 1, y), _mm256_store_si256(q + 2, z), _mm256_store_si256(q + 3, w);
 }
 std::memcpy(a, buf, n * sizeof(u32));
}
inline u32 median3(u32 a, u32 b, u32 c) { return std::max(std::min(a, b), std::min(std::max(a, b), c)); }
inline u32 choose_pivot(const u32* a, size_t n) {
 if(n < 128) return median3(a[0], a[n / 2], a[n - 1]);
 const size_t s= n / 8;
 return median3(median3(a[0], a[s], a[2 * s]), median3(a[3 * s], a[4 * s], a[5 * s]), median3(a[6 * s], a[7 * s], a[n - 1]));
}
inline void rec(u32* a, size_t n, int depth) {
 while(n > 32) {
  if(depth-- == 0) return std::sort(a, a + n);
  const u32 p= choose_pivot(a, n);
  size_t k= partition(a, n, p);
  if(k == n) {
   // 全要素が p 以下なので p は最大値。p と等しい要素 (並べ終わり) を右に分け、左だけを続ける。
   if(p == 0) return;
   n= partition(a, n, p - 1);
   continue;
  }
  // 小さい側を再帰で、大きい側をループで並べる (スタックの深さを log n に抑える)。
  if(k < n - k) rec(a, k, depth), a+= k, n-= k;
  else rec(a + k, n - k, depth), n= k;
 }
 small_sort(a, n);
}
inline void sort(std::vector<u32>& v) {
 int log= 0;
 for(size_t n= v.size(); n >>= 1;) ++log;
 rec(v.data(), v.size(), 2 * log);
}
}  // namespace sort_qsort_avx2
inline void run(std::vector<unsigned>& a) { sort_qsort_avx2::sort(a); }
