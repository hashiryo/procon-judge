#pragma once
// Berlekamp–Massey。値は M 未満の u32 で持ち、ずれの内積は AVX2 で u64 にためて 8 回ごとに 2^32 の位を畳み、
// 漸化式の更新 C -= c x^m B は掛ける数が決まっているので Shoup の方法で 8 個ずつ掛ける。
// 長さが伸びる回は、新しい C を B の置き場に後ろから書いて置き場を入れ替え、写しを作らない。
#ifdef __x86_64__
#include <immintrin.h>
#else
#include <simde/x86/avx2.h>
#endif
#include <algorithm>
#include <vector>
namespace bm_avx2 {
constexpr u32 M= 998244353;
constexpr u32 R32= u32((u64(1) << 32) % M);  // 2^32 mod M
inline u32 mulmod(u32 a, u32 b) { return u32(u64(a) * b % M); }
inline u32 inv(u32 a) {
 u32 r= 1;
 for(u32 e= M - 2; e; e>>= 1, a= mulmod(a, a))
  if(e & 1) r= mulmod(r, a);
 return r;
}
// 4 個の u64 それぞれの 2^32 の位を 2^32 mod M に掛けて畳む。2^62 + 2^32 未満になる。
inline __m256i fold(__m256i x) { return _mm256_add_epi64(_mm256_mul_epu32(_mm256_srli_epi64(x, 32), _mm256_set1_epi64x(R32)), _mm256_and_si256(x, _mm256_set1_epi64x(0xffffffffll))); }
// Σ x[i] y[i] mod M (x, y は M 未満)
inline u32 dot(const u32* x, const u32* y, int n) {
 __m256i a0= _mm256_setzero_si256(), a1= _mm256_setzero_si256();
 int i= 0;
 for(int t= 0; i + 8 <= n; i+= 8) {
  __m256i u= _mm256_loadu_si256((const __m256i*)(x + i)), v= _mm256_loadu_si256((const __m256i*)(y + i));
  a0= _mm256_add_epi64(a0, _mm256_mul_epu32(u, v));
  a1= _mm256_add_epi64(a1, _mm256_mul_epu32(_mm256_srli_epi64(u, 32), _mm256_srli_epi64(v, 32)));
  if(++t == 8) a0= fold(a0), a1= fold(a1), t= 0;
 }
 a0= _mm256_add_epi64(fold(a0), fold(a1));
 alignas(32) u64 s[4];
 _mm256_store_si256((__m256i*)s, a0);
 u64 r= s[0] % M + s[1] % M + s[2] % M + s[3] % M;
 for(; i < n; ++i) r+= u64(x[i]) * y[i] % M;
 return u32(r % M);
}
// a x mod M を 8 個ずつ (a < M、x < M、q = floor(x 2^32 / M))。結果は M 未満。
inline __m256i mul8(__m256i a, __m256i x, __m256i q, __m256i p) {
 __m256i ev= _mm256_srli_epi64(_mm256_mul_epu32(a, q), 32);
 __m256i od= _mm256_mul_epu32(_mm256_srli_epi64(a, 32), q);
 __m256i hi= _mm256_blend_epi32(ev, od, 0b10101010);
 __m256i r= _mm256_sub_epi32(_mm256_mullo_epi32(a, x), _mm256_mullo_epi32(hi, p));
 return _mm256_min_epu32(r, _mm256_sub_epi32(r, p));
}
inline u32 mul1(u32 a, u32 x, u32 q) {
 u32 r= a * x - u32((u64(a) * q) >> 32) * M;
 return r >= M ? r - M : r;
}
// d[i] = c[i] - x b[i] (i < n)。d と c は同じでよい。
inline void sub_mul(u32* d, const u32* c, const u32* b, int n, u32 x) {
 const u32 q= u32((u64(x) << 32) / M);
 const __m256i vx= _mm256_set1_epi32(x), vq= _mm256_set1_epi32(q), p= _mm256_set1_epi32(M);
 int i= 0;
 for(; i + 8 <= n; i+= 8) {
  __m256i r= _mm256_sub_epi32(_mm256_loadu_si256((const __m256i*)(c + i)), mul8(_mm256_loadu_si256((const __m256i*)(b + i)), vx, vq, p));
  _mm256_storeu_si256((__m256i*)(d + i), _mm256_min_epu32(r, _mm256_add_epi32(r, p)));
 }
 for(; i < n; ++i) {
  u32 r= c[i] - mul1(b[i], x, q);
  d[i]= int(r) < 0 ? r + M : r;
 }
}
// e[j] = c[j] - x e[j - m] (m ≤ j < m + n) を後ろから求める。e[j - m] を読んでから e[j] を書くので、8 個の塊ごとに
// 読みを先に済ませれば上書きの前に読める。
inline void sub_mul_back(u32* e, const u32* c, int m, int n, u32 x) {
 const u32 q= u32((u64(x) << 32) / M);
 const __m256i vx= _mm256_set1_epi32(x), vq= _mm256_set1_epi32(q), p= _mm256_set1_epi32(M);
 int j= m + n;
 for(; j - 8 >= m; j-= 8) {
  __m256i bb= _mm256_loadu_si256((const __m256i*)(e + j - 8 - m));
  __m256i r= _mm256_sub_epi32(_mm256_loadu_si256((const __m256i*)(c + j - 8)), mul8(bb, vx, vq, p));
  _mm256_storeu_si256((__m256i*)(e + j - 8), _mm256_min_epu32(r, _mm256_add_epi32(r, p)));
 }
 for(--j; j >= m; --j) {
  u32 r= c[j] - mul1(e[j - m], x, q);
  e[j]= int(r) < 0 ? r + M : r;
 }
}
}  // namespace bm_avx2
inline vector<u32> run(const vector<u32>& a) {
 using namespace bm_avx2;
 const int n= a.size();
 if(n == 0) return {};
 vector<u32> ar(a.rbegin(), a.rend());
 // C と B は長さの先が 0 になるように保つ。
 vector<u32> bufC(n + 9, 0), bufB(n + 9, 0);
 u32 *C= bufC.data(), *B= bufB.data();
 C[0]= B[0]= 1;
 int L= 0, m= 1, lc= 1, lb= 1;
 u32 binv= 1;
 for(int i= 0; i < n; ++i) {
  const u32 d= dot(C, ar.data() + (n - 1 - i), L + 1);
  if(d == 0) {
   ++m;
   continue;
  }
  const u32 x= mulmod(d, binv);
  if(2 * L <= i) {
   // 新しい C = C - x z^m B を B の置き場に作る。
   const int nl= std::max(lc, lb + m);
   sub_mul_back(B, C, m, lb, x);
   std::copy(C, C + m, B);
   if(lb + m < lc) std::copy(C + lb + m, C + lc, B + lb + m);
   std::swap(C, B);
   lb= lc, lc= nl;
   L= i + 1 - L, binv= inv(d), m= 1;
  } else {
   sub_mul(C + m, C + m, B, lb, x);
   lc= std::max(lc, lb + m);
   ++m;
  }
 }
 vector<u32> res(L);
 for(int j= 1; j <= L; ++j) res[j - 1]= C[j] ? M - C[j] : 0;
 return res;
}
