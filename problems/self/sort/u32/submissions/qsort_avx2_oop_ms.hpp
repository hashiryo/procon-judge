#pragma once
// qsort_avx2_oop の 32 個以下の塊を、作業用の配列への memcpy をやめ、端の半端なベクタを vpmaskmovd で読み書きする版。
// 葉の塊は 1e6 個で約 3 万あり、塊ごとの memcpy 2 回 (libc の呼び出し) が効いていると見ている。
// qsort_avx2 の分割を、その場で分けるのをやめて作業用の配列へ書く形にした版。8 個ずつ先頭から順に読み、軸以下を作業用の配列の
// 前から、軸より大きいものを後ろから詰める。読む位置が 1 か所なので、左右のどちらから読むかの選び分けも、両端を先に取って空きを
// 作る手間も要らない。分割のたびに a と作業用の配列の役を入れ替え、並べ終えた塊が作業用の配列の側にあれば a へ写す。軸の選び方、
// 重複の扱い、32 個以下のソーティングネットワークは qsort_avx2 と同じ。
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
#include <memory>
#include <vector>
namespace sort_qsort_avx2_oop_ms {
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
// x[0, n) を、p 以下を y の前から、p より大きいものを y の後ろから詰めて書き、前に書いた個数を返す。8 個ずつ読んで軸と比べ、
// movemask の 8 bit で表を引いて vpermd で並べ替え、同じベクタを y の前の書き込み位置と後ろの書き込み位置の両方に書く。
// 読む位置は先頭から順に進むだけなので、その場で分けるときの「左右のどちらから読むか」の選び分けが要らない。
inline size_t partition(const u32* x, u32* y, size_t n, u32 p) {
 const __m256i sign= _mm256_set1_epi32(int(0x80000000u));
 const __m256i pv= _mm256_set1_epi32(int(p ^ 0x80000000u));
 const __m256i shifts= _mm256_setr_epi32(0, 3, 6, 9, 12, 15, 18, 21);
 // [l, r) がまだ書いていない範囲で、幅は読んでいない個数と同じ。前の書き込み [l, l + 8) と後ろの書き込み [r - 8, r) が
 // 重なると、後ろの書き込みが前に置いた値を上書きするので、16 個以上残っている間だけベクタで進め、残りは 1 個ずつ置く。
 size_t l= 0, r= n, i= 0;
 for(; i + 16 <= n; i+= 8) {
  const __m256i v= _mm256_loadu_si256((const __m256i*)(x + i));
  const int m= _mm256_movemask_ps(_mm256_castsi256_ps(_mm256_cmpgt_epi32(_mm256_xor_si256(v, sign), pv)));
  const __m256i w= _mm256_permutevar8x32_epi32(v, _mm256_srlv_epi32(_mm256_set1_epi32(int(PERM.t[m])), shifts));
  const int nr= __builtin_popcount(m);
  _mm256_storeu_si256((__m256i*)(y + l), w);
  _mm256_storeu_si256((__m256i*)(y + r - 8), w);
  l+= 8 - nr, r-= nr;
 }
 for(; i < n; ++i) {
  if(x[i] <= p) y[l++]= x[i];
  else y[--r]= x[i];
 }
 return l;
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
// a[0, n) (n <= 32) を、8 個ずつのベクタに最大値で埋めて読み、8 / 16 / 32 個のネットワークで並べて書き戻す。端の半端なベクタは
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
inline void small_sort(u32* a, size_t n) {
 if(n <= 1) return;
 const int r= int(n);
 if(n <= 8) {
  store_part(a, r, sort8(load_pad(a, r)));
 } else if(n <= 16) {
  __m256i x= load_pad(a, 8), y= load_pad(a + 8, r - 8);
  sort16(x, y);
  store_part(a, 8, x), store_part(a + 8, r - 8, y);
 } else {
  __m256i x= load_pad(a, 8), y= load_pad(a + 8, 8), z= load_pad(a + 16, r - 16), w= load_pad(a + 24, r - 24);
  sort32(x, y, z, w);
  store_part(a, 8, x), store_part(a + 8, 8, y), store_part(a + 16, r - 16, z), store_part(a + 24, r - 24, w);
 }
}
inline u32 median3(u32 a, u32 b, u32 c) { return std::max(std::min(a, b), std::min(std::max(a, b), c)); }
inline u32 choose_pivot(const u32* a, size_t n) {
 if(n < 128) return median3(a[0], a[n / 2], a[n - 1]);
 const size_t s= n / 8;
 return median3(median3(a[0], a[s], a[2 * s]), median3(a[3 * s], a[4 * s], a[5 * s]), median3(a[6 * s], a[7 * s], a[n - 1]));
}
// x[0, n) を並べる。y は同じ長さの作業用の領域で、分割のたびに x と y を入れ替える。x_is_a は x が元の配列 a の側かで、
// 並べ終えた値は a の側に置く (a の側でなければ最後に y へ写す)。
inline void rec(u32* x, u32* y, size_t n, int depth, bool x_is_a) {
 while(n > 32) {
  if(depth-- == 0) {
   std::sort(x, x + n);
   if(!x_is_a) std::memcpy(y, x, n * sizeof(u32));
   return;
  }
  const u32 p= choose_pivot(x, n);
  size_t k= partition(x, y, n, p);
  std::swap(x, y), x_is_a= !x_is_a;
  if(k == n) {
   // 全要素が p 以下なので p は最大値。p と等しい要素 (並べ終わり) を後ろに分け、前だけを続ける。
   if(p == 0) {  // 全要素が 0
    if(!x_is_a) std::memcpy(y, x, n * sizeof(u32));
    return;
   }
   k= partition(x, y, n, p - 1);
   std::swap(x, y), x_is_a= !x_is_a;
   if(!x_is_a) std::memcpy(y + k, x + k, (n - k) * sizeof(u32));
   n= k;
   continue;
  }
  // 小さい側を再帰で、大きい側をループで並べる (スタックの深さを log n に抑える)。
  if(k < n - k) rec(x, y, k, depth, x_is_a), x+= k, y+= k, n-= k;
  else rec(x + k, y + k, n - k, depth, x_is_a), n= k;
 }
 small_sort(x, n);
 if(!x_is_a) std::memcpy(y, x, n * sizeof(u32));
}
inline void sort(std::vector<u32>& v) {
 const size_t n= v.size();
 if(n <= 32) return small_sort(v.data(), n);
 std::unique_ptr<u32[]> buf(new u32[n]);
 int log= 0;
 for(size_t t= n; t >>= 1;) ++log;
 rec(v.data(), buf.get(), n, 2 * log, true);
}
}  // namespace sort_qsort_avx2_oop_ms
inline void run(std::vector<unsigned>& a) { sort_qsort_avx2_oop_ms::sort(a); }
