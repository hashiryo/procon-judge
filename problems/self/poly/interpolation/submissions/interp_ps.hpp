#pragma once
// 係数の補間を、点ごとの組立除法でなく冪の和と畳み込み 1 回で求める。
// f = Σ_i w_i P(X)/(X - x_i) (P = Π (X - x_j)、w_i = y_i / P'(x_i)) の係数は、P/(X - x_i) の k 次の係数が
// Σ_{t>k} P_t x_i^{t-k-1} なので、f_k = Σ_{m=0}^{n-1-k} P_{k+1+m} s_m (s_m = Σ_i w_i x_i^m)。
// 1. P を (X - x_j) を 1 つずつ掛けて作る。掛ける数が決まっているので Shoup の方法で 8 個ずつ。
// 2. P'(x_i) を、点を 8 個ずつレーンに載せた Horner の方法で求める (4 本を並べて待ちを隠す)。値は Montgomery の形で持つ。
// 3. w_i の逆元はまとめて求める。
// 4. s_m は、点の組ごとに w x^m を 8 レーンで作り、m ごとの 8 レーンの和の配列に足す (4 組を並べる)。
// 5. f は P を逆に並べた列と s の畳み込みの下半分で、出力 32 個ずつの塊を u64 にためる素朴な積の核で求める。
#ifdef __x86_64__
#include <immintrin.h>
#else
#include <simde/x86/avx2.h>
#endif
#include <algorithm>
#include <vector>
namespace interp_ps {
constexpr u32 M= 998244353;
constexpr u32 MI= [] {
 u32 x= M;
 for(int i= 0; i < 5; ++i) x*= 2 - M * x;
 return -x;
}();
constexpr u32 R1= u32((u64(1) << 32) % M), R2= u32(u64(R1) * R1 % M);
inline u32 mul(u32 a, u32 b) { return u32(u64(a) * b % M); }
inline u32 pw(u32 a, u64 e) {
 u32 r= 1;
 for(; e; e>>= 1, a= mul(a, a))
  if(e & 1) r= mul(r, a);
 return r;
}
[[gnu::always_inline]] inline __m256i mont8(__m256i x, __m256i y) {
 const __m256i mv= _mm256_set1_epi32(M), mi= _mm256_set1_epi32(MI);
 const __m256i te= _mm256_mul_epu32(x, y), to= _mm256_mul_epu32(_mm256_srli_epi64(x, 32), _mm256_srli_epi64(y, 32));
 const __m256i ue= _mm256_add_epi64(te, _mm256_mul_epu32(_mm256_mul_epu32(te, mi), mv));
 const __m256i uo= _mm256_add_epi64(to, _mm256_mul_epu32(_mm256_mul_epu32(to, mi), mv));
 return _mm256_blend_epi32(_mm256_srli_epi64(ue, 32), uo, 0b10101010);
}
[[gnu::always_inline]] inline __m256i red8(__m256i x) { return _mm256_min_epu32(x, _mm256_sub_epi32(x, _mm256_set1_epi32(M))); }
// a x mod M を 8 個ずつ (a < M、x < M、q = floor(x 2^32 / M))。M 未満で返す。
[[gnu::always_inline]] inline __m256i shoup8(__m256i a, __m256i x, __m256i q) {
 const __m256i p= _mm256_set1_epi32(M);
 const __m256i ev= _mm256_srli_epi64(_mm256_mul_epu32(a, q), 32), od= _mm256_mul_epu32(_mm256_srli_epi64(a, 32), q);
 const __m256i r= _mm256_sub_epi32(_mm256_mullo_epi32(a, x), _mm256_mullo_epi32(_mm256_blend_epi32(ev, od, 0b10101010), p));
 return red8(r);
}
constexpr u32 R32= R1;
[[gnu::always_inline]] inline __m256i fold(__m256i x) { return _mm256_add_epi64(_mm256_mul_epu32(_mm256_srli_epi64(x, 32), _mm256_set1_epi64x(R32)), _mm256_and_si256(x, _mm256_set1_epi64x(0xffffffffll))); }
constexpr int PAD= 32;
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
}  // namespace interp_ps
inline vector<u32> run(const vector<u32>& x, const vector<u32>& y) {
 using namespace interp_ps;
 const int n= int(x.size());
 if(n <= 8) {  // 小さいときは素朴に
  vector<u64> P(n + 1, 0), f(n, 0), q(n);
  P[0]= 1;
  for(int j= 0; j < n; ++j) {
   for(int k= j + 1; k > 0; --k) P[k]= (P[k - 1] + (M - x[j]) * P[k]) % M;
   P[0]= (M - x[j]) * P[0] % M;
  }
  for(int i= 0; i < n; ++i) {
   q[n - 1]= P[n];
   for(int k= n - 1; k > 0; --k) q[k - 1]= (P[k] + u64(x[i]) * q[k]) % M;
   u64 d= 0;
   for(int k= n - 1; k >= 0; --k) d= (d * x[i] + q[k]) % M;
   const u64 w= y[i] * u64(pw(u32(d), M - 2)) % M;
   for(int k= 0; k < n; ++k) f[k]= (f[k] + w * q[k]) % M;
  }
  return vector<u32>(f.begin(), f.end());
 }
 // 1. P (長さ n + 1)。P_new[k] = P[k-1] - x_j P[k] を上から 8 個ずつ (読みを先に済ませる)。
 vector<u32> P(n + 1 + 16, 0);
 P[0]= 1;
 for(int j= 0; j < n; ++j) {
  const u32 c= x[j] ? M - x[j] : 0, q= u32((u64(c) << 32) / M);
  const __m256i vc= _mm256_set1_epi32(c), vq= _mm256_set1_epi32(q);
  int k= j + 2;               // P_new の添字は 0..j+1
  for(; k - 8 >= 0; k-= 8) {  // [k-8, k) を求める
   const int lo= k - 8;
   const __m256i cur= _mm256_loadu_si256((const __m256i*)(P.data() + lo));
   const __m256i prev= lo ? _mm256_loadu_si256((const __m256i*)(P.data() + lo - 1)) : _mm256_permutevar8x32_epi32(_mm256_and_si256(cur, _mm256_setr_epi32(-1, -1, -1, -1, -1, -1, -1, 0)), _mm256_setr_epi32(7, 0, 1, 2, 3, 4, 5, 6));
   _mm256_storeu_si256((__m256i*)(P.data() + lo), red8(_mm256_add_epi32(prev, shoup8(cur, vc, vq))));
  }
  for(--k; k >= 0; --k) P[k]= u32(((k ? P[k - 1] : 0) + u64(c) * P[k]) % M);
 }
 // 2. P'(x_i) (Montgomery の形の係数で Horner)。dP[k] = (k+1) P[k+1] R
 vector<u32> dPm(n);
 for(int k= 0; k < n; ++k) dPm[k]= mul(mul(u32(k + 1), P[k + 1]), R1);
 const int ng= (n + 7) / 8;
 vector<u32> xs(8 * ng + 32, 0), ds(8 * ng + 32, 1);
 for(int i= 0; i < n; ++i) xs[i]= x[i];
 const __m256i r2= _mm256_set1_epi32(R2);
 for(int g= 0; g < ng; g+= 4) {
  __m256i xm[4], d[4];
  for(int t= 0; t < 4; ++t) xm[t]= mont8(_mm256_loadu_si256((const __m256i*)(xs.data() + 8 * (g + t))), r2), d[t]= _mm256_setzero_si256();
  for(int k= n - 1; k >= 0; --k) {
   const __m256i c= _mm256_set1_epi32(dPm[k]);
   for(int t= 0; t < 4; ++t) d[t]= red8(_mm256_add_epi32(red8(mont8(d[t], xm[t])), c));
  }
  for(int t= 0; t < 4; ++t) _mm256_storeu_si256((__m256i*)(ds.data() + 8 * (g + t)), red8(mont8(d[t], _mm256_set1_epi32(1))));
 }
 // 3. w_i = y_i / P'(x_i) (逆元はまとめて)
 vector<u32> w(8 * ng + 32, 0), pre(n);
 {
  u32 acc= 1;
  for(int i= 0; i < n; ++i) pre[i]= acc, acc= mul(acc, ds[i]);
  u32 iv= pw(acc, M - 2);
  for(int i= n - 1; i >= 0; --i) w[i]= mul(mul(iv, pre[i]), y[i]), iv= mul(iv, ds[i]);
 }
 // 4. s_m = Σ_i w_i x_i^m (m < n)。点の組 4 つずつ、m ごとに 8 レーンの和を sv[m] に足す。
 vector<u32> sv(8 * n, 0);
 for(int g= 0; g < ng; g+= 4) {
  __m256i xm[4], v[4];
  for(int t= 0; t < 4; ++t) {
   xm[t]= mont8(_mm256_loadu_si256((const __m256i*)(xs.data() + 8 * (g + t))), r2);
   v[t]= _mm256_loadu_si256((const __m256i*)(w.data() + 8 * (g + t)));  // 本当の値 (組の外は 0)
  }
  for(int m= 0; m < n; ++m) {
   __m256i s= _mm256_loadu_si256((const __m256i*)(sv.data() + 8 * m));
   for(int t= 0; t < 4; ++t) s= red8(_mm256_add_epi32(s, v[t])), v[t]= red8(mont8(v[t], xm[t]));
   _mm256_storeu_si256((__m256i*)(sv.data() + 8 * m), s);
  }
 }
 vector<u32> s(n);
 for(int m= 0; m < n; ++m) {
  u64 t= 0;
  for(int b= 0; b < 8; ++b) t+= sv[8 * m + b];
  s[m]= u32(t % M);
 }
 // 5. f_k = Σ_m P_{k+1+m} s_m。Prev[j] = P_{n-j} (j < n) として f_k = (Prev * s)[n-1-k]。
 vector<u32> Prev(n), sp(n + 2 * PAD, 0), c(n + 32);
 for(int j= 0; j < n; ++j) Prev[j]= P[n - j];
 std::copy(s.begin(), s.end(), sp.begin() + PAD);
 for(int k0= 0; k0 < n; k0+= 32) conv32(Prev.data(), n, sp.data(), n, k0, c.data() + k0);
 vector<u32> f(n);
 for(int k= 0; k < n; ++k) f[k]= c[n - 1 - k];
 return f;
}
