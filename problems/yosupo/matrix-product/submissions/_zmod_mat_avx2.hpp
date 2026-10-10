#pragma once
#ifdef __x86_64__
#include <immintrin.h>
#else
#include <simde/x86/avx2.h>
#endif
#include "../common.hpp"
#include "neo/algebra/ZMod.hpp"
// _zmod_mat.hpp の mat_mul_acc と同じく、値を M 未満に直して u64 の和に積を足し、8 段ごとに 2^32 の位を 2^32 mod M に
// 掛けて畳む。和を 4 行 × 8 列のタイルとして 8 本の ymm に持ち、AVX2 の vpmuludq で 4 個ずつ掛ける (32 個の積和に
// vpmuludq が 8 回)。B は 8 列ずつの帯に詰め直して k の向きに続けて読み、A の 4 行は 1 個ずつ全レーンに配る。
// 帯を外側に回すので、帯 (m × 8 個) は L1 か L2 に残り、A を帯の数だけ読み直す。
template <class Z> vector<Z> mat_mul_avx2(int n, int m, int p, const vector<Z>& A, const vector<Z>& B) {
 const int n4= (n + 3) / 4 * 4, p8= (p + 7) / 8 * 8;
 vector<u32> a((size_t)n4 * m), b((size_t)p8 * m);  // b は帯ごとに [k][8] の順
 for(size_t i= 0; i < (size_t)n * m; ++i) a[i]= A[i].val();
 for(int k= 0; k < m; ++k)
  for(int j= 0; j < p; ++j) b[(size_t)(j & ~7) * m + k * 8 + (j & 7)]= B[(size_t)k * p + j].val();
 const __m256i R= _mm256_set1_epi64x((u64(1) << 32) % Z::mod()), LO= _mm256_set1_epi64x(0xffffffff);
 auto fold= [&](__m256i c) { return _mm256_add_epi64(_mm256_mul_epu32(_mm256_srli_epi64(c, 32), R), _mm256_and_si256(c, LO)); };
 vector<Z> C((size_t)n * p);
 alignas(32) u64 buf[4][8];
 for(int jb= 0; jb < p8; jb+= 8) {
  const u32* bp= &b[(size_t)jb * m];
  for(int i0= 0; i0 < n4; i0+= 4) {
   const u32 *a0= &a[(size_t)i0 * m], *a1= a0 + m, *a2= a1 + m, *a3= a2 + m;
   __m256i c00= _mm256_setzero_si256(), c01= c00, c10= c00, c11= c00, c20= c00, c21= c00, c30= c00, c31= c00;
   for(int k= 0; k < m;) {
    for(const int ke= min(m, k + 8); k < ke; ++k) {
     const __m256i b0= _mm256_cvtepu32_epi64(_mm_loadu_si128((const __m128i*)(bp + k * 8)));
     const __m256i b1= _mm256_cvtepu32_epi64(_mm_loadu_si128((const __m128i*)(bp + k * 8 + 4)));
     __m256i x= _mm256_set1_epi32(a0[k]);
     c00= _mm256_add_epi64(c00, _mm256_mul_epu32(x, b0)), c01= _mm256_add_epi64(c01, _mm256_mul_epu32(x, b1));
     x= _mm256_set1_epi32(a1[k]);
     c10= _mm256_add_epi64(c10, _mm256_mul_epu32(x, b0)), c11= _mm256_add_epi64(c11, _mm256_mul_epu32(x, b1));
     x= _mm256_set1_epi32(a2[k]);
     c20= _mm256_add_epi64(c20, _mm256_mul_epu32(x, b0)), c21= _mm256_add_epi64(c21, _mm256_mul_epu32(x, b1));
     x= _mm256_set1_epi32(a3[k]);
     c30= _mm256_add_epi64(c30, _mm256_mul_epu32(x, b0)), c31= _mm256_add_epi64(c31, _mm256_mul_epu32(x, b1));
    }
    if(k < m) {
     c00= fold(c00), c01= fold(c01), c10= fold(c10), c11= fold(c11);
     c20= fold(c20), c21= fold(c21), c30= fold(c30), c31= fold(c31);
    }
   }
   _mm256_store_si256((__m256i*)&buf[0][0], c00), _mm256_store_si256((__m256i*)&buf[0][4], c01);
   _mm256_store_si256((__m256i*)&buf[1][0], c10), _mm256_store_si256((__m256i*)&buf[1][4], c11);
   _mm256_store_si256((__m256i*)&buf[2][0], c20), _mm256_store_si256((__m256i*)&buf[2][4], c21);
   _mm256_store_si256((__m256i*)&buf[3][0], c30), _mm256_store_si256((__m256i*)&buf[3][4], c31);
   for(int r= 0; r < 4 && i0 + r < n; ++r)
    for(int j= 0; j < 8 && jb + j < p; ++j) C[(size_t)(i0 + r) * p + jb + j]= Z(buf[r][j]);
  }
 }
 return C;
}
