#pragma once
// mod 998244353 の w × w の行列の積 C = A B 2^{-32} を、4 行 × 8 列のタイルで求める核 (w は 8 の倍数)。
// 値を Montgomery の表現 (x 2^32 mod M) で持てば、積もそのまま Montgomery の表現になるので、累乗のあいだは変換が要らない。
// 核の中身は matrix-product の _strassen.hpp の 64 × 64 の核と同じで、大きさを w に広げ、B を 8 列ずつの帯に詰め直して
// k の向きに続けて読む。B の行の 8 個を 1 回読み、偶数番目はそのまま、奇数番目は 32 bit 右にずらして vpmuludq に渡し、
// u64 の和 8 本 (4 行 × 偶奇) に足す。8 段ごとに、最上位の bit が立っていれば 2M 2^32 を引いて 2^63 未満に保ち
// (M と互いに素な 2^32 の倍数なので、最後の Montgomery の還元の値は変わらない)、最後に Montgomery の還元で 2^32 で割る。
#include "../common.hpp"
#ifndef USE_SIMDE
#include <immintrin.h>
#endif
namespace tile {
constexpr u32 M= MOD;
constexpr u32 NIV= [] {  // -M^{-1} mod 2^32
 u32 x= M;
 for(int i= 0; i < 5; ++i) x*= 2 - M * x;
 return -x;
}();
constexpr u32 R1= u32((u64(1) << 32) % M);    // 1 の Montgomery の表現
constexpr u32 R2= u32((u128(1) << 64) % M);  // 2^64 mod M (Montgomery の表現に直すときに掛ける)
inline u32 redc(u64 x) {                     // x 2^{-32} mod M (x < 2^32 M)
 const u32 q= u32(x) * NIV;
 const u32 r= u32((x + u64(q) * M) >> 32);
 return r >= M ? r - M : r;
}
inline u32 to_mont(u32 x) { return redc(u64(x) * R2); }
inline __m256i shrk(__m256i x, __m256i m) { return _mm256_min_epu32(x, _mm256_sub_epi32(x, m)); }
// 偶数番目の和 e と奇数番目の和 o (どちらも 2^63 未満) を Montgomery の還元で 2^32 で割り、8 個の u32 (4M 未満) に並べる。
inline __m256i redc8(__m256i e, __m256i o) {
 const __m256i niv= _mm256_set1_epi32(NIV), m= _mm256_set1_epi32(M);
 const __m256i ke= _mm256_mul_epu32(_mm256_mul_epu32(e, niv), m), ko= _mm256_mul_epu32(_mm256_mul_epu32(o, niv), m);
 return _mm256_blend_epi32(_mm256_srli_epi64(_mm256_add_epi64(e, ke), 32), _mm256_add_epi64(o, ko), 0xaa);
}
// B (w × w、行優先) を 8 列ずつの帯 [帯][k][8] に詰め直す。
inline void pack(const u32* b, u32* bp, int w) {
 for(int jb= 0; jb < w; jb+= 8)
  for(int k= 0; k < w; ++k) memcpy(bp + (size_t)jb * w + k * 8, b + (size_t)k * w + jb, 8 * sizeof(u32));
}
// c = a b 2^{-32} mod M。a、c は w × w の行優先、bp は pack(b) の結果。値は M 未満。
inline void mul_packed(const u32* __restrict a, const u32* __restrict bp, u32* __restrict c, int w) {
 const __m256i Mv= _mm256_set1_epi32(M), M2= _mm256_set1_epi32(2 * M), TOP= _mm256_set1_epi64x(1ll << 63);
 auto keep= [&](__m256i x) { return _mm256_sub_epi64(x, _mm256_min_epu32(M2, _mm256_and_si256(TOP, x))); };
 for(int jb= 0; jb < w; jb+= 8) {
  const u32* band= bp + (size_t)jb * w;
  for(int i= 0; i < w; i+= 4) {
   const u32 *a0= a + (size_t)i * w, *a1= a0 + w, *a2= a1 + w, *a3= a2 + w;
   __m256i e0= _mm256_setzero_si256(), o0= e0, e1= e0, o1= e0, e2= e0, o2= e0, e3= e0, o3= e0;
   for(int k= 0; k < w; k+= 8) {
    for(int t= k; t < k + 8; ++t) {
     const __m256i z= _mm256_loadu_si256((const __m256i*)(band + t * 8)), zo= _mm256_srli_epi64(z, 32);
     __m256i x= _mm256_set1_epi32(a0[t]);
     e0= _mm256_add_epi64(e0, _mm256_mul_epu32(x, z)), o0= _mm256_add_epi64(o0, _mm256_mul_epu32(x, zo));
     x= _mm256_set1_epi32(a1[t]);
     e1= _mm256_add_epi64(e1, _mm256_mul_epu32(x, z)), o1= _mm256_add_epi64(o1, _mm256_mul_epu32(x, zo));
     x= _mm256_set1_epi32(a2[t]);
     e2= _mm256_add_epi64(e2, _mm256_mul_epu32(x, z)), o2= _mm256_add_epi64(o2, _mm256_mul_epu32(x, zo));
     x= _mm256_set1_epi32(a3[t]);
     e3= _mm256_add_epi64(e3, _mm256_mul_epu32(x, z)), o3= _mm256_add_epi64(o3, _mm256_mul_epu32(x, zo));
    }
    e0= keep(e0), o0= keep(o0), e1= keep(e1), o1= keep(o1), e2= keep(e2), o2= keep(o2), e3= keep(e3), o3= keep(o3);
   }
   _mm256_storeu_si256((__m256i*)(c + (size_t)i * w + jb), shrk(shrk(redc8(e0, o0), M2), Mv));
   _mm256_storeu_si256((__m256i*)(c + (size_t)(i + 1) * w + jb), shrk(shrk(redc8(e1, o1), M2), Mv));
   _mm256_storeu_si256((__m256i*)(c + (size_t)(i + 2) * w + jb), shrk(shrk(redc8(e2, o2), M2), Mv));
   _mm256_storeu_si256((__m256i*)(c + (size_t)(i + 3) * w + jb), shrk(shrk(redc8(e3, o3), M2), Mv));
  }
 }
}
// 行列を w × w (w は n を 8 の倍数に切り上げたもの) に 0 で広げ、Montgomery の表現に直す。
inline vector<u32> load(int n, int w, const vector<u32>& a) {
 vector<u32> x((size_t)w * w);
 for(int i= 0; i < n; ++i)
  for(int j= 0; j < n; ++j) x[(size_t)i * w + j]= to_mont(a[(size_t)i * n + j]);
 return x;
}
inline vector<u32> store(int n, int w, const vector<u32>& x) {
 vector<u32> a((size_t)n * n);
 for(int i= 0; i < n; ++i)
  for(int j= 0; j < n; ++j) a[(size_t)i * n + j]= redc(x[(size_t)i * w + j]);
 return a;
}
// A^K を、上の bit から 2 乗と積を重ねて求める (A は Montgomery の表現、w × w)。
inline vector<u32> pow(const vector<u32>& A, u64 K, int w) {
 vector<u32> r((size_t)w * w), t((size_t)w * w), bp((size_t)w * w), ap((size_t)w * w);
 if(K == 0) {
  for(int i= 0; i < w; ++i) r[(size_t)i * w + i]= R1;
  return r;
 }
 pack(A.data(), ap.data(), w);
 r= A;
 for(int b= 62 - __builtin_clzll(K); b >= 0; --b) {
  pack(r.data(), bp.data(), w), mul_packed(r.data(), bp.data(), t.data(), w), swap(r, t);
  if(K >> b & 1) mul_packed(r.data(), ap.data(), t.data(), w), swap(r, t);
 }
 return r;
}
}  // namespace tile
