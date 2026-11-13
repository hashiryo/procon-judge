#pragma once
// 確かめ用。NeoLibrary 60945a2 の linalg_internal::zmat の Strassen の道 (s_kernel、s_mul、s_place、s_unplace、strassen_mul) を、
// ブロックの 1 辺 SB をテンプレートの引数にして写したもの。核のタイル、畳み込み、Winograd の並べ方は元のまま。
#include "../common.hpp"
#include "neo/linalg/Matrix.hpp"
namespace neo_sb {
using namespace linalg_internal::zmat;
using linalg_internal::Region;
template <int SB> [[gnu::noinline]] void s_kernel(const u32* __restrict a, const u32* __restrict b, u32* __restrict c, const Mod& md) {
 const __m256i M1= _mm256_set1_epi32(md.m), M2= _mm256_set1_epi32(2 * md.m), TOP= _mm256_set1_epi64x((long long)(1ull << 63));
 const __m256i NIV= _mm256_set1_epi32(md.niv), R32= _mm256_set1_epi64x(md.r32), LO= _mm256_set1_epi64x(0xffffffffll);
 for(int i= 0; i < SB; i+= 4) {
  const u32 *a0= a + i * SB, *a1= a0 + SB, *a2= a1 + SB, *a3= a2 + SB;
  for(int j= 0; j < SB; j+= 8) {
   __m256i e0= _mm256_setzero_si256(), o0= e0, e1= e0, o1= e0, e2= e0, o2= e0, e3= e0, o3= e0;
   for(int k= 0; k < SB; k+= 8) {
    for(int w= k; w < k + 8; ++w) {
     const __m256i z= _mm256_loadu_si256((const __m256i*)(b + w * SB + j)), zo= odd_lanes(b + w * SB + j, z);
     __m256i x= _mm256_set1_epi32(a0[w]);
     e0= _mm256_add_epi64(e0, _mm256_mul_epu32(x, z)), o0= _mm256_add_epi64(o0, _mm256_mul_epu32(x, zo));
     x= _mm256_set1_epi32(a1[w]);
     e1= _mm256_add_epi64(e1, _mm256_mul_epu32(x, z)), o1= _mm256_add_epi64(o1, _mm256_mul_epu32(x, zo));
     x= _mm256_set1_epi32(a2[w]);
     e2= _mm256_add_epi64(e2, _mm256_mul_epu32(x, z)), o2= _mm256_add_epi64(o2, _mm256_mul_epu32(x, zo));
     x= _mm256_set1_epi32(a3[w]);
     e3= _mm256_add_epi64(e3, _mm256_mul_epu32(x, z)), o3= _mm256_add_epi64(o3, _mm256_mul_epu32(x, zo));
    }
    e0= keep(e0, M2, TOP), o0= keep(o0, M2, TOP), e1= keep(e1, M2, TOP), o1= keep(o1, M2, TOP);
    e2= keep(e2, M2, TOP), o2= keep(o2, M2, TOP), e3= keep(e3, M2, TOP), o3= keep(o3, M2, TOP);
   }
   _mm256_storeu_si256((__m256i*)(c + i * SB + j), shrk(redc8(fold(e0, R32, LO), fold(o0, R32, LO), NIV, M1), M1));
   _mm256_storeu_si256((__m256i*)(c + (i + 1) * SB + j), shrk(redc8(fold(e1, R32, LO), fold(o1, R32, LO), NIV, M1), M1));
   _mm256_storeu_si256((__m256i*)(c + (i + 2) * SB + j), shrk(redc8(fold(e2, R32, LO), fold(o2, R32, LO), NIV, M1), M1));
   _mm256_storeu_si256((__m256i*)(c + (i + 3) * SB + j), shrk(redc8(fold(e3, R32, LO), fold(o3, R32, LO), NIV, M1), M1));
  }
 }
}
inline void s_add(u32* x, const u32* y, const u32* z, std::size_t d, __m256i m) {
 for(std::size_t i= 0; i < d; i+= 8) _mm256_storeu_si256((__m256i*)(x + i), shrk(_mm256_add_epi32(_mm256_loadu_si256((const __m256i*)(y + i)), _mm256_loadu_si256((const __m256i*)(z + i))), m));
}
inline void s_sub(u32* x, const u32* y, const u32* z, std::size_t d, __m256i m) {
 for(std::size_t i= 0; i < d; i+= 8) _mm256_storeu_si256((__m256i*)(x + i), dilt(_mm256_sub_epi32(_mm256_loadu_si256((const __m256i*)(y + i)), _mm256_loadu_si256((const __m256i*)(z + i))), m));
}
template <int SB> void s_mul(const u32* a, const u32* b, u32* c, std::size_t n, u32* X, u32* Y, const Mod& md) {
 if(n == SB) return s_kernel<SB>(a, b, c, md);
 const __m256i m= _mm256_set1_epi32(md.m);
 const std::size_t h= n / 2, d= h * h;
 const u32 *a11= a, *a12= a + d, *a21= a + 2 * d, *a22= a + 3 * d;
 const u32 *b11= b, *b12= b + d, *b21= b + 2 * d, *b22= b + 3 * d;
 u32 *c11= c, *c12= c + d, *c21= c + 2 * d, *c22= c + 3 * d, *X2= X + d, *Y2= Y + d;
 s_sub(X, a11, a21, d, m), s_sub(Y, b22, b12, d, m), s_mul<SB>(X, Y, c21, h, X2, Y2, md);
 s_add(X, a21, a22, d, m), s_sub(Y, b12, b11, d, m), s_mul<SB>(X, Y, c22, h, X2, Y2, md);
 s_sub(X, X, a11, d, m), s_sub(Y, b22, Y, d, m), s_mul<SB>(X, Y, c12, h, X2, Y2, md);
 s_sub(X, a12, X, d, m), s_mul<SB>(X, b22, c11, h, X2, Y2, md);
 s_mul<SB>(a11, b11, X, h, X2, Y2, md);
 s_add(c12, X, c12, d, m), s_add(c21, c12, c21, d, m), s_add(c12, c12, c22, d, m), s_add(c22, c21, c22, d, m), s_add(c12, c12, c11, d, m);
 s_sub(Y, Y, b21, d, m), s_mul<SB>(a22, Y, c11, h, X2, Y2, md), s_sub(c21, c21, c11, d, m);
 s_mul<SB>(a12, b21, c11, h, X2, Y2, md), s_add(c11, X, c11, d, m);
}
template <int SB, class Z> void s_place(const Z* src, int rows, int cols, std::size_t ld, std::size_t n, u32* dst) {
 if(n == SB) {
  for(int i= 0; i < SB; ++i) {
   u32* d= dst + i * SB;
   const int w= i < rows ? std::clamp(cols, 0, SB) : 0;
   for(int j= 0; j < w; ++j) d[j]= src[i * ld + j].val();
   std::fill(d + w, d + SB, 0u);
  }
  return;
 }
 const std::size_t h= n / 2, d= h * h;
 s_place<SB>(src, rows, cols, ld, h, dst);
 s_place<SB>(src + h, rows, int(cols - h), ld, h, dst + d);
 s_place<SB>(src + h * ld, int(rows - h), cols, ld, h, dst + 2 * d);
 s_place<SB>(src + h * ld + h, int(rows - h), int(cols - h), ld, h, dst + 3 * d);
}
template <int SB, class Z> void s_unplace(const u32* src, int rows, int cols, std::size_t ld, std::size_t n, Z* dst, const Mod& md) {
 if(rows <= 0 || cols <= 0) return;
 if(n == SB) {
  const __m256i r2= _mm256_set1_epi64x(md.r2), m= _mm256_set1_epi32(md.m), niv= _mm256_set1_epi32(md.niv);
  alignas(32) u32 t[8];
  for(int i= 0; i < std::min(rows, SB); ++i)
   for(int j= 0; j < std::min(cols, SB); j+= 8) {
    const __m256i x= _mm256_loadu_si256((const __m256i*)(src + i * SB + j));
    const __m256i y= shrk(redc8(_mm256_mul_epu32(x, r2), _mm256_mul_epu32(_mm256_srli_epi64(x, 32), r2), niv, m), m);
    if(j + 8 <= cols) _mm256_storeu_si256((__m256i*)(dst + i * ld + j), y);
    else {
     _mm256_store_si256((__m256i*)t, y);
     for(int q= 0; j + q < cols; ++q) dst[i * ld + j + q]= Z::raw(t[q]);
    }
   }
  return;
 }
 const std::size_t h= n / 2, d= h * h;
 s_unplace<SB>(src, rows, cols, ld, h, dst, md);
 s_unplace<SB>(src + d, rows, int(cols - h), ld, h, dst + h, md);
 s_unplace<SB>(src + 2 * d, int(rows - h), cols, ld, h, dst + h * ld, md);
 s_unplace<SB>(src + 3 * d, int(rows - h), int(cols - h), ld, h, dst + h * ld + h, md);
}
// 元の strassen_side と同じ見積もり (積和 7^d SB^3 の 1.25 倍とタイルの n p m を比べる)。
template <int SB> std::size_t strassen_side(int n, int m, int p) {
 std::size_t N= SB, c= std::size_t(SB) * SB * SB;
 while(N < std::size_t(std::max({n, m, p}))) N*= 2, c*= 7;
 const double tile= double((n + 3) & ~3) * ((m + 7) & ~7) * ((p + 7) & ~7);
 return N >= 4 * SB && 1.25 * double(c) < tile ? N : 0;
}
template <int SB, class Z> void strassen_mul(const Z* a, const Z* b, Z* c, int n, int m, int p, std::size_t N, const Mod& md) {
 const std::size_t NN= N * N, W= NN / 3 + SB * SB;
 Region R(3 * NN + 2 * W);
 u32 *A= R.p, *B= A + NN, *C= B + NN, *X= C + NN, *Y= X + W;
 s_place<SB>(a, n, m, m, N, A), s_place<SB>(b, m, p, p, N, B);
 s_mul<SB>(A, B, C, N, X, Y, md);
 s_unplace<SB>(C, n, p, p, N, c, md);
}
// 元の zmat::mul の Strassen を選ぶ分岐だけを SB で置き換えたもの。
template <int SB, class Z> void mul(const Z* A, const Z* B, Z* C, int n, int m, int p) {
 if(n == 0 || p == 0) return;
 const double vol= double(n) * m * p, padded= double((n + 3) & ~3) * ((m + 7) & ~7) * ((p + 7) & ~7);
 if(vol < 128 || padded > 2.5 * vol) return small_mul(A, B, C, n, m, p);
 const Mod md(Z::mod());
 if(md.odd)
  if(const std::size_t N= strassen_side<SB>(n, m, p)) return strassen_mul<SB>(A, B, C, n, m, p, N, md);
 tile_mul<Z>(A, B, C, n, m, p, md);
}
}  // namespace neo_sb
