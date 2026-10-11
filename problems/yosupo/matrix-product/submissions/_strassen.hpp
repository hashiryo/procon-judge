#pragma once
// mod 998244353 の行列積を、64 × 64 のブロックの核の上に Strassen の再帰 (7 回の積) を重ねて求める。
// 形は yosupo_fastest (提出 193471) と同じで、行列を 1 辺が 2 の冪の正方形に 0 で広げ、64 × 64 のブロックを Morton 順に
// 並べ直してから再帰する。記録は algo-notes の notes/linear-algebra.md。
//
// 核は C = A B 2^{-32} (mod M) を 4 行 × 8 列のタイルで求める。B の行の 8 個を 1 回読み、偶数番目はそのまま、奇数番目は
// 32 bit 右にずらして vpmuludq に渡し、u64 の和 8 本 (4 行 × 偶奇) に足す。8 段ごとに、最上位の bit が立っていれば
// 2M 2^32 を引いて 2^63 未満に保ち (M と互いに素な 2^32 の倍数なので、最後の Montgomery の還元の値は変わらない)、
// 最後に Montgomery の還元で 2^32 で割る。再帰の中は足し算と引き算だけなので、どのブロックも同じ 2^{-32} 倍を持ち、
// 出力のときに 2^64 mod M を Montgomery の掛け算で掛けて戻す。
#include "../common.hpp"
#ifndef USE_SIMDE
#include <immintrin.h>
#endif
namespace strassen {
constexpr u32 M= MOD;
constexpr u32 NIV= [] {  // -M^{-1} mod 2^32
 u32 x= M;
 for(int i= 0; i < 5; ++i) x*= 2 - M * x;
 return -x;
}();
constexpr u32 R2= u32((u128(1) << 64) % M);
constexpr int BLK= 64;
inline __m256i shrk(__m256i x, __m256i m) { return _mm256_min_epu32(x, _mm256_sub_epi32(x, m)); }
inline __m256i dilt(__m256i x, __m256i m) { return _mm256_min_epu32(x, _mm256_add_epi32(x, m)); }
// 偶数番目の和 e と奇数番目の和 o (どちらも 2^63 未満) を Montgomery の還元で 2^32 で割り、8 個の u32 (4M 未満) に並べる。
inline __m256i redc(__m256i e, __m256i o) {
 const __m256i niv= _mm256_set1_epi32(NIV), m= _mm256_set1_epi32(M);
 const __m256i ke= _mm256_mul_epu32(_mm256_mul_epu32(e, niv), m), ko= _mm256_mul_epu32(_mm256_mul_epu32(o, niv), m);
 return _mm256_blend_epi32(_mm256_srli_epi64(_mm256_add_epi64(e, ke), 32), _mm256_add_epi64(o, ko), 0xaa);
}
// c = a b 2^{-32} mod M (64 × 64 のブロック、値は M 未満)。
[[gnu::noinline]] inline void kernel(const u32* __restrict a, const u32* __restrict b, u32* __restrict c) {
 const __m256i Mv= _mm256_set1_epi32(M), M2= _mm256_set1_epi32(2 * M), TOP= _mm256_set1_epi64x(1ll << 63);
 auto keep= [&](__m256i x) { return _mm256_sub_epi64(x, _mm256_min_epu32(M2, _mm256_and_si256(TOP, x))); };
 for(int i= 0; i < BLK; i+= 4) {
  const u32 *a0= a + i * BLK, *a1= a0 + BLK, *a2= a1 + BLK, *a3= a2 + BLK;
  for(int j= 0; j < BLK; j+= 8) {
   __m256i e0= _mm256_setzero_si256(), o0= e0, e1= e0, o1= e0, e2= e0, o2= e0, e3= e0, o3= e0;
   for(int k= 0; k < BLK; k+= 8) {
    for(int w= k; w < k + 8; ++w) {
     const __m256i z= _mm256_loadu_si256((const __m256i*)(b + w * BLK + j)), zo= _mm256_srli_epi64(z, 32);
     __m256i x= _mm256_set1_epi32(a0[w]);
     e0= _mm256_add_epi64(e0, _mm256_mul_epu32(x, z)), o0= _mm256_add_epi64(o0, _mm256_mul_epu32(x, zo));
     x= _mm256_set1_epi32(a1[w]);
     e1= _mm256_add_epi64(e1, _mm256_mul_epu32(x, z)), o1= _mm256_add_epi64(o1, _mm256_mul_epu32(x, zo));
     x= _mm256_set1_epi32(a2[w]);
     e2= _mm256_add_epi64(e2, _mm256_mul_epu32(x, z)), o2= _mm256_add_epi64(o2, _mm256_mul_epu32(x, zo));
     x= _mm256_set1_epi32(a3[w]);
     e3= _mm256_add_epi64(e3, _mm256_mul_epu32(x, z)), o3= _mm256_add_epi64(o3, _mm256_mul_epu32(x, zo));
    }
    e0= keep(e0), o0= keep(o0), e1= keep(e1), o1= keep(o1), e2= keep(e2), o2= keep(o2), e3= keep(e3), o3= keep(o3);
   }
   _mm256_storeu_si256((__m256i*)(c + i * BLK + j), shrk(shrk(redc(e0, o0), M2), Mv));
   _mm256_storeu_si256((__m256i*)(c + (i + 1) * BLK + j), shrk(shrk(redc(e1, o1), M2), Mv));
   _mm256_storeu_si256((__m256i*)(c + (i + 2) * BLK + j), shrk(shrk(redc(e2, o2), M2), Mv));
   _mm256_storeu_si256((__m256i*)(c + (i + 3) * BLK + j), shrk(shrk(redc(e3, o3), M2), Mv));
  }
 }
}
inline void add(u32* x, const u32* y, const u32* z, size_t d) {
 const __m256i m= _mm256_set1_epi32(M);
 for(size_t i= 0; i < d; i+= 8)
  _mm256_storeu_si256((__m256i*)(x + i), shrk(_mm256_add_epi32(_mm256_loadu_si256((const __m256i*)(y + i)), _mm256_loadu_si256((const __m256i*)(z + i))), m));
}
inline void sub(u32* x, const u32* y, const u32* z, size_t d) {
 const __m256i m= _mm256_set1_epi32(M);
 for(size_t i= 0; i < d; i+= 8)
  _mm256_storeu_si256((__m256i*)(x + i), dilt(_mm256_sub_epi32(_mm256_loadu_si256((const __m256i*)(y + i)), _mm256_loadu_si256((const __m256i*)(z + i))), m));
}
// n × n のブロック列 (Morton 順で、4 つの象限が続けて並ぶ) の積 c = a b 2^{-32}。c の後ろの n^2 / 4 + ... を作業に使う。
// aa、bb は象限の和を置く作業領域で、深い段はその後ろを使う。
inline void mul(const u32* a, const u32* b, u32* c, size_t n, u32* aa, u32* bb) {
 if(n == BLK) return kernel(a, b, c);
 const size_t h= n / 2, d= h * h;
 const u32 *a00= a, *a01= a + d, *a10= a + 2 * d, *a11= a + 3 * d;
 const u32 *b00= b, *b01= b + d, *b10= b + 2 * d, *b11= b + 3 * d;
 u32 *c00= c, *c01= c + d, *c10= c + 2 * d, *c11= c + 3 * d, *cc= c + n * n;
 // C00 = M7 - M5 + M4 + M1、C01 = M5 + M3、C10 = M4 + M2、C11 = M6 - M2 + M1 + M3 (Strassen の 7 つの積)
 sub(aa, a01, a11, d), add(bb, b10, b11, d), mul(aa, bb, c00, h, aa + d, bb + d);  // M7
 add(aa, a00, a01, d), mul(aa, b11, c01, h, aa + d, bb + d), sub(c00, c00, c01, d);  // M5
 sub(bb, b10, b00, d), mul(a11, bb, c10, h, aa + d, bb + d), add(c00, c00, c10, d);  // M4
 sub(aa, a10, a00, d), add(bb, b00, b01, d), mul(aa, bb, c11, h, aa + d, bb + d);  // M6
 add(aa, a10, a11, d), mul(aa, b00, cc, h, aa + d, bb + d), add(c10, c10, cc, d), sub(c11, c11, cc, d);  // M2
 add(aa, a00, a11, d), add(bb, b00, b11, d), mul(aa, bb, cc, h, aa + d, bb + d), add(c00, c00, cc, d), add(c11, c11, cc, d);  // M1
 sub(bb, b01, b11, d), mul(a00, bb, cc, h, aa + d, bb + d), add(c01, c01, cc, d), add(c11, c11, cc, d);  // M3
}
// 行 rows、列 cols の行列 src (行の間隔 ld) を、1 辺 n のブロック列 (Morton 順) に写す。範囲の外は 0。
inline void place(const u32* src, int rows, int cols, size_t ld, size_t n, u32* dst) {
 if(n == BLK) {
  for(int i= 0; i < BLK; ++i) {
   u32* d= dst + i * BLK;
   if(i < rows) {
    const int w= min(cols, BLK);
    if(w > 0) memcpy(d, src + i * ld, w * sizeof(u32));
    if(w < BLK) memset(d + max(w, 0), 0, (BLK - max(w, 0)) * sizeof(u32));
   } else memset(d, 0, BLK * sizeof(u32));
  }
  return;
 }
 const size_t h= n / 2, d= h * h;
 place(src, rows, cols, ld, h, dst);
 place(src + h, rows, (int)(cols - h), ld, h, dst + d);
 place(src + h * ld, (int)(rows - h), cols, ld, h, dst + 2 * d);
 place(src + h * ld + h, (int)(rows - h), (int)(cols - h), ld, h, dst + 3 * d);
}
// ブロック列 src (1 辺 n、値は a b 2^{-32}) に 2^64 mod M を Montgomery の掛け算で掛け、行列 dst (行の間隔 ld) の範囲内に書く。
inline void unplace(const u32* src, int rows, int cols, size_t ld, size_t n, u32* dst) {
 if(rows <= 0 || cols <= 0) return;
 if(n == BLK) {
  const __m256i r2= _mm256_set1_epi32(R2), m= _mm256_set1_epi32(M);
  alignas(32) u32 t[8];
  for(int i= 0; i < min(rows, BLK); ++i)
   for(int j= 0; j < min(cols, BLK); j+= 8) {
    const __m256i x= _mm256_loadu_si256((const __m256i*)(src + i * BLK + j));
    const __m256i y= shrk(redc(_mm256_mul_epu32(x, r2), _mm256_mul_epu32(_mm256_srli_epi64(x, 32), r2)), m);
    if(j + 8 <= cols) _mm256_storeu_si256((__m256i*)(dst + i * ld + j), y);
    else {
     _mm256_store_si256((__m256i*)t, y);
     for(int q= 0; j + q < cols; ++q) dst[i * ld + j + q]= t[q];
    }
   }
  return;
 }
 const size_t h= n / 2, d= h * h;
 unplace(src, rows, cols, ld, h, dst);
 unplace(src + d, rows, (int)(cols - h), ld, h, dst + h);
 unplace(src + 2 * d, (int)(rows - h), cols, ld, h, dst + h * ld);
 unplace(src + 3 * d, (int)(rows - h), (int)(cols - h), ld, h, dst + h * ld + h);
}
inline vector<u32> matmul(int n, int m, int p, const vector<u32>& a, const vector<u32>& b) {
 size_t N= BLK;
 while(N < (size_t)max({n, m, p})) N*= 2;
 const size_t NN= N * N, W= NN / 3 + BLK * BLK;  // 作業領域は n^2 / 4 + n^2 / 16 + ... < n^2 / 3
 vector<u32> A(NN + W), B(NN + W), C(NN + W);
 place(a.data(), n, m, m, N, A.data());
 place(b.data(), m, p, p, N, B.data());
 mul(A.data(), B.data(), C.data(), N, A.data() + NN, B.data() + NN);
 vector<u32> res((size_t)n * p);
 unplace(C.data(), n, p, p, N, res.data());
 return res;
}
}  // namespace strassen
