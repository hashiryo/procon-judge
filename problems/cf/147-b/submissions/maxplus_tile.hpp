#pragma once
// int の max-plus の行列の積を、4 行 × 16 列の和を 8 本の ymm に持つタイルで求める。B は 16 列の帯ごとに k の向きに読み (帯は
// 300 行で 19 KB なので L1 に残る)、k ごとに B の 2 本を読み、A の 4 個を広げて、vpaddd と vpmaxsd を 8 本ずつ行う。行列は行を
// 4 の倍数に、列を 16 の倍数 w に NEG で広げて持つ。値の範囲は maxplus_ikj と同じで、NEG = -2^29 以上に保つ。手順は reference と同じ。
#include "../common.hpp"
#ifndef USE_SIMDE
#include <immintrin.h>
#endif
namespace mp_tile {
constexpr int NEG= -(1 << 29);
// z = x y。x、y、z は h4 行 × w 列 (h4 は 4 の倍数、w は 16 の倍数)、掛ける長さは n。
inline void mul(const int* __restrict x, const int* __restrict y, int* __restrict z, int n, int h4, int w) {
 const __m256i NG= _mm256_set1_epi32(NEG);
 for(int j0= 0; j0 < w; j0+= 16)
  for(int i0= 0; i0 < h4; i0+= 4) {
   const int *x0= x + (size_t)i0 * w, *x1= x0 + w, *x2= x1 + w, *x3= x2 + w;
   const int* yb= y + j0;
   __m256i a00= NG, a01= NG, a10= NG, a11= NG, a20= NG, a21= NG, a30= NG, a31= NG;
   for(int k= 0; k < n; ++k) {
    const __m256i y0= _mm256_loadu_si256((const __m256i*)(yb + (size_t)k * w)), y1= _mm256_loadu_si256((const __m256i*)(yb + (size_t)k * w + 8));
    __m256i b= _mm256_set1_epi32(x0[k]);
    a00= _mm256_max_epi32(a00, _mm256_add_epi32(b, y0)), a01= _mm256_max_epi32(a01, _mm256_add_epi32(b, y1));
    b= _mm256_set1_epi32(x1[k]);
    a10= _mm256_max_epi32(a10, _mm256_add_epi32(b, y0)), a11= _mm256_max_epi32(a11, _mm256_add_epi32(b, y1));
    b= _mm256_set1_epi32(x2[k]);
    a20= _mm256_max_epi32(a20, _mm256_add_epi32(b, y0)), a21= _mm256_max_epi32(a21, _mm256_add_epi32(b, y1));
    b= _mm256_set1_epi32(x3[k]);
    a30= _mm256_max_epi32(a30, _mm256_add_epi32(b, y0)), a31= _mm256_max_epi32(a31, _mm256_add_epi32(b, y1));
   }
   int* zr= z + (size_t)i0 * w + j0;
   _mm256_storeu_si256((__m256i*)zr, _mm256_max_epi32(a00, NG)), _mm256_storeu_si256((__m256i*)(zr + 8), _mm256_max_epi32(a01, NG));
   zr+= w;
   _mm256_storeu_si256((__m256i*)zr, _mm256_max_epi32(a10, NG)), _mm256_storeu_si256((__m256i*)(zr + 8), _mm256_max_epi32(a11, NG));
   zr+= w;
   _mm256_storeu_si256((__m256i*)zr, _mm256_max_epi32(a20, NG)), _mm256_storeu_si256((__m256i*)(zr + 8), _mm256_max_epi32(a21, NG));
   zr+= w;
   _mm256_storeu_si256((__m256i*)zr, _mm256_max_epi32(a30, NG)), _mm256_storeu_si256((__m256i*)(zr + 8), _mm256_max_epi32(a31, NG));
  }
}
}  // namespace mp_tile
inline int solve(int n, const vector<array<int, 4>>& doors) {
 using mp_tile::NEG;
 const int h4= (n + 3) & ~3, w= (n + 15) & ~15;
 using Mat= vector<int>;
 auto unit= [&] {
  Mat m((size_t)h4 * w, NEG);
  for(int i= 0; i < n; ++i) m[(size_t)i * w + i]= 0;
  return m;
 };
 Mat a= unit();
 for(auto [i, j, c, d]: doors) a[(size_t)i * w + j]= max(a[(size_t)i * w + j], c), a[(size_t)j * w + i]= max(a[(size_t)j * w + i], d);
 int lg= 0;
 while((1 << lg) <= n) ++lg;
 vector<Mat> P(lg, Mat((size_t)h4 * w));
 P[0]= a;
 for(int k= 1; k < lg; ++k) mp_tile::mul(P[k - 1].data(), P[k - 1].data(), P[k].data(), n, h4, w);
 Mat cur= unit(), t((size_t)h4 * w);
 int len= 0;
 for(int k= lg - 1; k >= 0; --k) {
  if(len + (1 << k) > n) continue;
  mp_tile::mul(cur.data(), P[k].data(), t.data(), n, h4, w);
  bool pos= false;
  for(int i= 0; i < n; ++i) pos|= t[(size_t)i * w + i] > 0;
  if(!pos) cur.swap(t), len+= 1 << k;
 }
 return len >= n ? 0 : len + 1;
}
