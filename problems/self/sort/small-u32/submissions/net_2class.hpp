#pragma once
// net_bl の組み分けを 2 つに減らした版。32 個以下は 32 個のネットワーク、33 個から 64 個は 64 個のネットワークで並べる。
// 組み分けの分岐が 1 つで済む代わりに、短い配列でも 32 個分の手間がかかる。読み書きは全部 vpmaskmovd で、分岐しない。
// 64 個を超える配列は std::sort に任せる。
#ifdef USE_SIMDE
#include <simde/x86/avx2.h>
#else
#include <immintrin.h>
#endif
#include <algorithm>
#include <vector>
namespace small_net_2class {
using u32= unsigned;
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
// a から始まる 8 個のうち、先頭の rem 個 (rem は 0 以下や 8 以上でもよい) を vpmaskmovd で読み書きする。読むときは残りの lane を
// 最大値で埋める。8 個そろっているかで分岐しないので、大きさが毎回変わっても分岐の予測が外れない。
inline __m256i lane_mask(int rem) { return _mm256_cmpgt_epi32(_mm256_set1_epi32(rem), _mm256_setr_epi32(0, 1, 2, 3, 4, 5, 6, 7)); }
inline __m256i load_bl(const u32* a, int rem) {
 const __m256i m= lane_mask(rem);
 return _mm256_blendv_epi8(_mm256_set1_epi32(-1), _mm256_maskload_epi32((const int*)a, m), m);
}
inline void store_bl(u32* a, int rem, __m256i v) { _mm256_maskstore_epi32((int*)a, lane_mask(rem), v); }
inline void sort_small(u32* a, int n) {
 if(n <= 32) {
  __m256i v[4];
  for(int t= 0; t < 4; ++t) v[t]= load_bl(a + 8 * t, n - 8 * t);
  sort32(v[0], v[1], v[2], v[3]);
  for(int t= 0; t < 4; ++t) store_bl(a + 8 * t, n - 8 * t, v[t]);
 } else {
  __m256i v[8];
  for(int t= 0; t < 8; ++t) v[t]= load_bl(a + 8 * t, n - 8 * t);
  sort64(v);
  for(int t= 0; t < 8; ++t) store_bl(a + 8 * t, n - 8 * t, v[t]);
 }
}
inline void sort(std::vector<u32>& v) {
 const size_t n= v.size();
 if(n <= 1) return;
 if(n <= 64) return sort_small(v.data(), int(n));
 std::sort(v.begin(), v.end());
}
}  // namespace small_net_2class
inline void run(std::vector<unsigned>& a) { small_net_2class::sort(a); }
