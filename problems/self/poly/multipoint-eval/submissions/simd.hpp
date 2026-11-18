#pragma once
// SIMD の実装。32 点 (8 レーンの 4 本) を並べて Horner 法を回す。点 x を掛けるのは Shoup の方法で、商 x 2^32 / M を
// レーンごとに先に作る (上位 32 bit は偶数と奇数のレーンを _mm256_mul_epu32 で別に求めて混ぜる)。値は 2M 未満のまま回し、
// 最後に M 未満にする。32 点に満たない残りは 8 点ずつ、さらに残りはスカラー。
#ifdef __x86_64__
#include <immintrin.h>
#else
#include <simde/x86/avx2.h>
#endif
#include <vector>
namespace mpe_simd {
constexpr u32 M= 998244353;
// a x mod M (a < 2^32、x < M、q = floor(x 2^32 / M)) を 2M 未満で
inline __m256i shoup8(__m256i a, __m256i x, __m256i q, __m256i mv) {
 const __m256i ev= _mm256_srli_epi64(_mm256_mul_epu32(a, q), 32), od= _mm256_mul_epu32(_mm256_srli_epi64(a, 32), _mm256_srli_epi64(q, 32));
 const __m256i hi= _mm256_blend_epi32(ev, od, 0b10101010);
 return _mm256_sub_epi32(_mm256_mullo_epi32(a, x), _mm256_mullo_epi32(hi, mv));
}
inline __m256i quo8(const u32* x) {
 alignas(32) u32 q[8];
 for(int k= 0; k < 8; ++k) q[k]= u32((u64(x[k]) << 32) / M);
 return _mm256_load_si256((const __m256i*)q);
}
}  // namespace mpe_simd
inline vector<u32> run(const vector<u32>& f, const vector<u32>& x) {
 using namespace mpe_simd;
 const int n= int(f.size()), m= int(x.size());
 vector<u32> r(m);
 const __m256i mv= _mm256_set1_epi32(M), m2= _mm256_set1_epi32(2 * M);
 auto step= [&](__m256i a, __m256i p, __m256i q, __m256i c) {
  const __m256i v= _mm256_add_epi32(shoup8(a, p, q, mv), c);  // 3M 未満
  return _mm256_min_epu32(v, _mm256_sub_epi32(v, m2));
 };
 auto fin= [&](__m256i a) { return _mm256_min_epu32(a, _mm256_sub_epi32(a, mv)); };
 int j= 0;
 for(; j + 32 <= m; j+= 32) {
  const __m256i p0= _mm256_loadu_si256((const __m256i*)(x.data() + j)), p1= _mm256_loadu_si256((const __m256i*)(x.data() + j + 8));
  const __m256i p2= _mm256_loadu_si256((const __m256i*)(x.data() + j + 16)), p3= _mm256_loadu_si256((const __m256i*)(x.data() + j + 24));
  const __m256i q0= quo8(x.data() + j), q1= quo8(x.data() + j + 8), q2= quo8(x.data() + j + 16), q3= quo8(x.data() + j + 24);
  __m256i a0= _mm256_setzero_si256(), a1= a0, a2= a0, a3= a0;
  for(int i= n - 1; i >= 0; --i) {
   const __m256i c= _mm256_set1_epi32(int(f[i]));
   a0= step(a0, p0, q0, c), a1= step(a1, p1, q1, c), a2= step(a2, p2, q2, c), a3= step(a3, p3, q3, c);
  }
  _mm256_storeu_si256((__m256i*)(r.data() + j), fin(a0)), _mm256_storeu_si256((__m256i*)(r.data() + j + 8), fin(a1));
  _mm256_storeu_si256((__m256i*)(r.data() + j + 16), fin(a2)), _mm256_storeu_si256((__m256i*)(r.data() + j + 24), fin(a3));
 }
 for(; j + 8 <= m; j+= 8) {
  const __m256i p= _mm256_loadu_si256((const __m256i*)(x.data() + j)), q= quo8(x.data() + j);
  __m256i a= _mm256_setzero_si256();
  for(int i= n - 1; i >= 0; --i) a= step(a, p, q, _mm256_set1_epi32(int(f[i])));
  _mm256_storeu_si256((__m256i*)(r.data() + j), fin(a));
 }
 for(; j < m; ++j) {
  const u32 p= x[j], q= u32((u64(p) << 32) / M);
  u32 a= 0;
  for(int i= n - 1; i >= 0; --i) {
   u32 v= a * p - u32((u64(a) * q) >> 32) * M + f[i];
   a= v >= 2 * M ? v - 2 * M : v;
  }
  r[j]= a >= M ? a - M : a;
 }
 return r;
}
