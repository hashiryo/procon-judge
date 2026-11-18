#pragma once
// Bostan–Mori の方法。Q = E(x^2) + x O(x^2)、P = Pe(x^2) + x Po(x^2) と分けて、半分の大きさの積 4 回で 1 段進める
// (Q(x) Q(-x) = E(y)^2 - y O(y)^2、P(x) Q(-x) の偶数部 = Pe E - y Po O、奇数部 = Po E - Pe O)。
// 積は短い側が 16 以下ならスカラーで u64 にため、それより大きければ AVX2 で出力 32 個ずつの塊を u64 にためる
// (8 回ごとに 2^32 の位を畳む)。作業の配列は呼び出しをまたいで使い回す。
#ifdef __x86_64__
#include <immintrin.h>
#else
#include <simde/x86/avx2.h>
#endif
#include <algorithm>
#include <vector>
namespace bmo {
constexpr u32 M= 998244353;
constexpr u32 R32= u32((u64(1) << 32) % M);
inline u32 mulm(u32 a, u32 b) { return u32(u64(a) * b % M); }
inline u32 pw(u32 a, u64 e) {
 u32 r= 1;
 for(; e; e>>= 1, a= mulm(a, a))
  if(e & 1) r= mulm(r, a);
 return r;
}
inline __m256i fold(__m256i x) { return _mm256_add_epi64(_mm256_mul_epu32(_mm256_srli_epi64(x, 32), _mm256_set1_epi64x(R32)), _mm256_and_si256(x, _mm256_set1_epi64x(0xffffffffll))); }
constexpr int PAD= 32;
// out[t] = Σ_i a[i] b[k0 + t - i] mod M (0 ≤ t < 32)。bp は b の前後に PAD 個の 0 を置いたもの。
inline void conv32(const u32* a, int n, const u32* bp, int m, int k0, u32* out) {
 const int ilo= std::max(0, k0 - m + 1), ihi= std::min(n - 1, k0 + 31);
 __m256i e0= _mm256_setzero_si256(), e1= e0, e2= e0, e3= e0, o0= e0, o1= e0, o2= e0, o3= e0;
 for(int i= ilo; i <= ihi;) {
  const int iend= std::min(ihi + 1, i + 8);
  for(; i < iend; ++i) {
   const __m256i x= _mm256_set1_epi32(a[i]);
   const u32* q= bp + PAD + k0 - i;
   const __m256i b0= _mm256_loadu_si256((const __m256i*)q), b1= _mm256_loadu_si256((const __m256i*)(q + 8));
   const __m256i b2= _mm256_loadu_si256((const __m256i*)(q + 16)), b3= _mm256_loadu_si256((const __m256i*)(q + 24));
   e0= _mm256_add_epi64(e0, _mm256_mul_epu32(x, b0)), o0= _mm256_add_epi64(o0, _mm256_mul_epu32(x, _mm256_srli_epi64(b0, 32)));
   e1= _mm256_add_epi64(e1, _mm256_mul_epu32(x, b1)), o1= _mm256_add_epi64(o1, _mm256_mul_epu32(x, _mm256_srli_epi64(b1, 32)));
   e2= _mm256_add_epi64(e2, _mm256_mul_epu32(x, b2)), o2= _mm256_add_epi64(o2, _mm256_mul_epu32(x, _mm256_srli_epi64(b2, 32)));
   e3= _mm256_add_epi64(e3, _mm256_mul_epu32(x, b3)), o3= _mm256_add_epi64(o3, _mm256_mul_epu32(x, _mm256_srli_epi64(b3, 32)));
  }
  e0= fold(e0), e1= fold(e1), e2= fold(e2), e3= fold(e3), o0= fold(o0), o1= fold(o1), o2= fold(o2), o3= fold(o3);
 }
 alignas(32) u64 ev[16], od[16];
 _mm256_store_si256((__m256i*)ev, e0), _mm256_store_si256((__m256i*)(ev + 4), e1), _mm256_store_si256((__m256i*)(ev + 8), e2), _mm256_store_si256((__m256i*)(ev + 12), e3);
 _mm256_store_si256((__m256i*)od, o0), _mm256_store_si256((__m256i*)(od + 4), o1), _mm256_store_si256((__m256i*)(od + 8), o2), _mm256_store_si256((__m256i*)(od + 12), o3);
 for(int j= 0; j < 16; ++j) out[2 * j]= u32(ev[j] % M), out[2 * j + 1]= u32(od[j] % M);
}
struct Work {
 std::vector<u32> pad, blk;
};
// out[0 .. n+m-1) = a b (M 未満)。
inline void mul(const u32* a, int n, const u32* b, int m, u32* out, Work& w) {
 if(!n || !m) return;
 const int len= n + m - 1;
 if(std::min(n, m) <= 16) {
  if(n > m) std::swap(a, b), std::swap(n, m);
  for(int k= 0; k < len; ++k) {
   u64 s= 0;
   const int lo= std::max(0, k - m + 1), hi= std::min(n - 1, k);
   for(int i= lo; i <= hi; ++i) s+= u64(a[i]) * b[k - i];
   out[k]= u32(s % M);
  }
  return;
 }
 w.pad.assign(m + 2 * PAD, 0);
 std::copy(b, b + m, w.pad.begin() + PAD);
 u32 tmp[32];
 for(int k0= 0; k0 < len; k0+= 32) {
  conv32(a, n, w.pad.data(), m, k0, tmp);
  std::copy(tmp, tmp + std::min(32, len - k0), out + k0);
 }
}
inline u32 sub(u32 x, u32 y) { return x >= y ? x - y : x + M - y; }
}  // namespace bmo
inline u32 run(const vector<u32>& a, const vector<u32>& c, u64 k) {
 using namespace bmo;
 const int d= int(a.size());
 if(k < u64(d)) return a[k];
 if(d == 1) return mulm(a[0], pw(c[0], k));
 static thread_local std::vector<u32> Q, P, E, O, Pe, Po, T1, T2;
 static thread_local Work w;
 Q.assign(d + 1, 0), Q[0]= 1;
 for(int j= 1; j <= d; ++j) Q[j]= c[j - 1] ? M - c[j - 1] : 0;
 T1.resize(2 * d + 1);
 mul(a.data(), d, Q.data(), d + 1, T1.data(), w);
 P.assign(T1.begin(), T1.begin() + d);
 const int ne= d / 2 + 1, no= (d + 1) / 2, npe= (d + 1) / 2, npo= d / 2;
 E.resize(ne), O.resize(no), Pe.resize(npe), Po.resize(npo), T2.resize(2 * d + 1);
 for(; k; k>>= 1) {
  for(int i= 0; i < ne; ++i) E[i]= Q[2 * i];
  for(int i= 0; i < no; ++i) O[i]= Q[2 * i + 1];
  for(int i= 0; i < npe; ++i) Pe[i]= P[2 * i];
  for(int i= 0; i < npo; ++i) Po[i]= P[2 * i + 1];
  mul(E.data(), ne, E.data(), ne, T1.data(), w);
  mul(O.data(), no, O.data(), no, T2.data(), w);
  Q[0]= T1[0];
  for(int i= 1; i <= d; ++i) Q[i]= sub(i < 2 * ne - 1 ? T1[i] : 0, i - 1 < 2 * no - 1 ? T2[i - 1] : 0);
  if(k & 1) {
   mul(Po.data(), npo, E.data(), ne, T1.data(), w);
   mul(Pe.data(), npe, O.data(), no, T2.data(), w);
   for(int i= 0; i < d; ++i) P[i]= sub(i < npo + ne - 1 ? T1[i] : 0, i < npe + no - 1 ? T2[i] : 0);
  } else {
   mul(Pe.data(), npe, E.data(), ne, T1.data(), w);
   mul(Po.data(), npo, O.data(), no, T2.data(), w);
   P[0]= T1[0];
   for(int i= 1; i < d; ++i) P[i]= sub(i < npe + ne - 1 ? T1[i] : 0, i - 1 < npo + no - 1 ? T2[i - 1] : 0);
  }
 }
 return P[0];
}
