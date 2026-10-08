#pragma once
// net_avx2 の半端なベクタの読み書きを、vpmaskmovd を使わずに、配列の最後の 8 個をふつうに読み書きして lane を回す形にした版。
// 8 個以上なら最後の 8 個はいつでも配列の中にあるので、読むときは最後の 8 個を読んで端のベクタに入る分を下の lane へ回し、書くときは
// 並べ終えた最後の 8 個を前のベクタと端のベクタから回して作って書く。8 個未満はベクタを読めないので挿入ソートにする。64 個を超える
// 配列は std::sort に任せる。ネットワークは net_avx2 と同じ。
#ifdef USE_SIMDE
#include <simde/x86/avx2.h>
#else
#include <immintrin.h>
#endif
#include <algorithm>
#include <vector>
namespace small_net_avx2_ov {
using u32= unsigned;
// 挿入ソート (短い配列向け)。
inline void insertion(u32* a, size_t n) {
 for(size_t i= 1; i < n; ++i) {
  const u32 x= a[i];
  size_t j= i;
  for(; j > 0 && a[j - 1] > x; --j) a[j]= a[j - 1];
  a[j]= x;
 }
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
// a, b, c, d を並べた 32 個が bitonic なら昇順にする。
inline void merge32(__m256i& a, __m256i& b, __m256i& c, __m256i& d) {
 __m256i l0= vmin(a, c), h0= vmax(a, c), l1= vmin(b, d), h1= vmax(b, d);
 merge16(l0, l1), merge16(h0, h1);
 a= l0, b= l1, c= h0, d= h1;
}
// 8 本のベクタの 64 個を昇順にする。前後の 32 個を並べ、後ろを逆順にしてつないだ bitonic な 64 個を merge する。
inline void sort64(__m256i* v) {
 sort32(v[0], v[1], v[2], v[3]), sort32(v[4], v[5], v[6], v[7]);
 const __m256i r4= reverse8(v[7]), r5= reverse8(v[6]), r6= reverse8(v[5]), r7= reverse8(v[4]);
 __m256i l0= vmin(v[0], r4), h0= vmax(v[0], r4), l1= vmin(v[1], r5), h1= vmax(v[1], r5);
 __m256i l2= vmin(v[2], r6), h2= vmax(v[2], r6), l3= vmin(v[3], r7), h3= vmax(v[3], r7);
 merge32(l0, l1, l2, l3), merge32(h0, h1, h2, h3);
 v[0]= l0, v[1]= l1, v[2]= l2, v[3]= l3, v[4]= h0, v[5]= h1, v[6]= h2, v[7]= h3;
}
// a[0, n) (n <= 64) を、8 個ずつのベクタに最大値で埋めて読み、8 / 16 / 32 / 64 個のネットワークで並べて書き戻す。端の半端なベクタは
// vpmaskmovd で読み書きし、配列の外には触れない (作業用の配列と memcpy を使わない)。rem はそのベクタから先に残っている個数。
inline __m256i load_pad(const u32* a, int rem) {
 if(rem >= 8) return _mm256_loadu_si256((const __m256i*)a);
 const __m256i m= _mm256_cmpgt_epi32(_mm256_set1_epi32(rem), _mm256_setr_epi32(0, 1, 2, 3, 4, 5, 6, 7));
 return _mm256_blendv_epi8(_mm256_set1_epi32(-1), _mm256_maskload_epi32((const int*)a, m), m);
}
inline void store_part(u32* a, int rem, __m256i v) {
 if(rem >= 8) return _mm256_storeu_si256((__m256i*)a, v);
 const __m256i m= _mm256_cmpgt_epi32(_mm256_set1_epi32(rem), _mm256_setr_epi32(0, 1, 2, 3, 4, 5, 6, 7));
 _mm256_maskstore_epi32((int*)a, m, v);
}
// src[0, n) (n <= 64) を並べて dst[0, n) に書く (src と dst は同じでもよい)。
inline void small_sort_to(const u32* src, u32* dst, size_t n) {
 if(n <= 1) {
  if(n) dst[0]= src[0];
  return;
 }
 const int r= int(n);
 if(n <= 8) {
  store_part(dst, r, sort8(load_pad(src, r)));
 } else if(n <= 16) {
  __m256i x= load_pad(src, 8), y= load_pad(src + 8, r - 8);
  sort16(x, y);
  store_part(dst, 8, x), store_part(dst + 8, r - 8, y);
 } else if(n <= 32) {
  __m256i x= load_pad(src, 8), y= load_pad(src + 8, 8), z= load_pad(src + 16, r - 16), w= load_pad(src + 24, r - 24);
  sort32(x, y, z, w);
  store_part(dst, 8, x), store_part(dst + 8, 8, y), store_part(dst + 16, r - 16, z), store_part(dst + 24, r - 24, w);
 } else {
  __m256i v[8];
  for(int j= 0; j < 8; ++j) v[j]= load_pad(src + 8 * j, r - 8 * j);
  sort64(v);
  for(int j= 0; j < 8; ++j) store_part(dst + 8 * j, r - 8 * j, v[j]);
 }
}
inline void small_sort(u32* a, size_t n) { small_sort_to(a, a, n); }
// v の lane を k だけ下へ回す (lane j に lane (j + k) mod 8 を置く)。vpermd は添字の下の 3 bit だけを見る。
inline __m256i rot(__m256i v, __m256i idx) { return _mm256_permutevar8x32_epi32(v, idx); }
// a[0, n) (8 <= n <= 64、8 (NV / 2) < n <= 8 NV) を NV 本のベクタのネットワークで並べる。q は 8 個そろったベクタの本数、c (1 から 8) は
// その次の端のベクタに入る個数。n >= 8 なので、最後の 8 個 a[n - 8, n) はいつでもふつうに読み書きできる。読むときは最後の 8 個を読み、
// 端のベクタに入る c 個を下の lane へ回してから、残りの lane を最大値で埋める。書くときは、並べ終えた最後の 8 個を、端の 1 つ前の
// ベクタの上の 8 - c 個と端のベクタの下の c 個から回して作り、a[n - 8, n) に書く (前のベクタと重なる所は同じ値を書き直すだけ)。
template <int NV> inline void sort_ov(u32* a, int n) {
 const int q= (n - 1) >> 3, c= n - 8 * q;
 const __m256i pad= _mm256_set1_epi32(-1), iota= _mm256_setr_epi32(0, 1, 2, 3, 4, 5, 6, 7);
 const __m256i in_idx= _mm256_add_epi32(iota, _mm256_set1_epi32(8 - c)), out_idx= _mm256_add_epi32(iota, _mm256_set1_epi32(c));
 const __m256i in_mask= _mm256_cmpgt_epi32(_mm256_set1_epi32(c), iota), out_mask= _mm256_cmpgt_epi32(iota, _mm256_set1_epi32(7 - c));
 __m256i v[NV];
#pragma GCC unroll 8
 for(int t= 0; t < NV; ++t) {
  if(t < q) v[t]= _mm256_loadu_si256((const __m256i*)(a + 8 * t));
  else if(t == q) v[t]= _mm256_blendv_epi8(pad, rot(_mm256_loadu_si256((const __m256i*)(a + n - 8)), in_idx), in_mask);
  else v[t]= pad;
 }
 if constexpr(NV == 1) v[0]= sort8(v[0]);
 else if constexpr(NV == 2) sort16(v[0], v[1]);
 else if constexpr(NV == 4) sort32(v[0], v[1], v[2], v[3]);
 else sort64(v);
#pragma GCC unroll 8
 for(int t= 0; t < NV; ++t) {
  if(t < q) {
   _mm256_storeu_si256((__m256i*)(a + 8 * t), v[t]);
  } else if(t == q) {
   if constexpr(NV == 1) _mm256_storeu_si256((__m256i*)a, v[0]);
   else _mm256_storeu_si256((__m256i*)(a + n - 8), t ? _mm256_blendv_epi8(rot(v[t ? t - 1 : 0], out_idx), rot(v[t], out_idx), out_mask) : v[0]);
  }
 }
}
inline void sort(std::vector<u32>& v) {
 u32* a= v.data();
 const size_t n= v.size();
 if(n < 8) return insertion(a, n);
 if(n == 8) return sort_ov<1>(a, 8);
 if(n <= 16) return sort_ov<2>(a, int(n));
 if(n <= 32) return sort_ov<4>(a, int(n));
 if(n <= 64) return sort_ov<8>(a, int(n));
 std::sort(v.begin(), v.end());
}
}  // namespace small_net_avx2_ov
inline void run(std::vector<unsigned>& a) { small_net_avx2_ov::sort(a); }
