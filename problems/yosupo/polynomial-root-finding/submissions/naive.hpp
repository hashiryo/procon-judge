#pragma once
// FFT を使わない根の列挙。多項式の積は素朴な O(nm) で、AVX2 で出力 32 個ずつの塊を u64 にためる (8 回ごとに 2^32 の位を
// 畳む)。f を法とする剰余は、x^N に関する逆元を先に作っておく Barrett の方法で、積の下半分を 2 回求める。
// 根は Cantor–Zassenhaus の方法で分ける。h = x^{(p-1)/2} mod f から gcd(f, h - 1) と gcd(f, h + 1) で 0 でない根を
// 平方剰余とそうでないものに分け、それぞれを乱択の a による gcd(g, (x + a)^{(p-1)/2} - 1) で 1 次になるまで割る。
#ifdef __x86_64__
#include <immintrin.h>
#else
#include <simde/x86/avx2.h>
#endif
#include <algorithm>
#include <random>
#include <vector>
namespace rf1 {
using poly= std::vector<u32>;
constexpr u32 M= 998244353;
constexpr u32 R32= u32((u64(1) << 32) % M);
inline u32 mul(u32 a, u32 b) { return u32(u64(a) * b % M); }
inline u32 add(u32 a, u32 b) { return a + b >= M ? a + b - M : a + b; }
inline u32 sub(u32 a, u32 b) { return a >= b ? a - b : a + M - b; }
inline u32 pw(u32 a, u64 e) {
 u32 r= 1;
 for(; e; e>>= 1, a= mul(a, a))
  if(e & 1) r= mul(r, a);
 return r;
}
inline u32 inv(u32 a) { return pw(a, M - 2); }
inline void trim(poly& a) {
 while(!a.empty() && a.back() == 0) a.pop_back();
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
// (a b) の 0 次から len - 1 次まで。
inline poly mul_low(const poly& a, const poly& b, int len) {
 const int n= int(a.size()), m= int(b.size());
 if(!n || !m || len <= 0) return {};
 len= std::min(len, n + m - 1);
 poly bp(m + 2 * PAD), c(len + 32);
 std::copy(b.begin(), b.end(), bp.begin() + PAD);
 for(int k0= 0; k0 < len; k0+= 32) conv32(a.data(), n, bp.data(), m, k0, c.data() + k0);
 c.resize(len);
 return c;
}
inline poly mul(const poly& a, const poly& b) { return mul_low(a, b, int(a.size() + b.size()) - 1); }
// monic な f を法とする剰余。inv = rev(f)^{-1} mod x^{N-1}。
struct ModF {
 poly f, inv_rev;
 int N;
 explicit ModF(const poly& f_): f(f_), N(int(f_.size()) - 1) {
  // rev(f) = Σ f[N - j] x^j、定数項は 1。逆元を素朴に前から。
  const int L= std::max(N - 1, 1);
  inv_rev.assign(L, 0), inv_rev[0]= 1;
  for(int k= 1; k < L; ++k) {
   u64 s= 0;
   for(int j= 1, t= 0; j <= std::min(k, N); ++j) {
    s+= u64(f[N - j]) * inv_rev[k - j];
    if(++t == 16) s%= M, t= 0;
   }
   inv_rev[k]= s % M ? M - u32(s % M) : 0;
  }
 }
 poly reduce(poly c) const {
  trim(c);
  if(int(c.size()) <= N) return c;
  const int dc= int(c.size()) - 1, L= dc - N + 1;
  poly cr(L);
  for(int i= 0; i < L; ++i) cr[i]= c[dc - i];
  poly qr= mul_low(cr, inv_rev, L);
  qr.resize(L);
  std::reverse(qr.begin(), qr.end());
  const poly qf= mul_low(qr, f, N);
  poly r(N);
  for(int k= 0; k < N; ++k) r[k]= sub(c[k], k < int(qf.size()) ? qf[k] : 0);
  trim(r);
  return r;
 }
 poly mulmod(const poly& a, const poly& b) const { return reduce(mul(a, b)); }
 // a (x + s) mod f
 poly mul_linear(const poly& a, u32 s) const {
  poly r(a.size() + 1);
  for(size_t i= 0; i < a.size(); ++i) r[i + 1]= add(r[i + 1], a[i]), r[i]= add(r[i], mul(a[i], s));
  trim(r);
  if(int(r.size()) > N) {
   const u32 lc= r[N];
   for(int i= 0; i < N; ++i) r[i]= sub(r[i], mul(lc, f[i]));
   r.resize(N), trim(r);
  }
  return r;
 }
 // (x + s)^e mod f
 poly pow_linear(u32 s, u64 e) const {
  poly r{1};
  for(int b= 63; b >= 0; --b) {
   if(r.size() > 1 || (r.size() == 1 && r[0] != 1)) r= mulmod(r, r);
   if(e >> b & 1) r= mul_linear(r, s);
  }
  return r;
 }
};
// a mod b (b は monic)
inline void rem_monic(poly& a, const poly& b) {
 const int db= int(b.size()) - 1;
 for(int i= int(a.size()) - 1; i >= db; --i) {
  const u32 c= a[i];
  if(!c) continue;
  const u32 x= M - c, q= u32((u64(x) << 32) / M);
  u32* p= a.data() + i - db;
  for(int j= 0; j < db; ++j) {
   u32 t= b[j] * x - u32((u64(b[j]) * q) >> 32) * M;  // b[j] (M - c) mod M を 2M 未満で
   t= t >= M ? t - M : t;
   p[j]= add(p[j], t);
  }
  a[i]= 0;
 }
 a.resize(std::min<size_t>(a.size(), db)), trim(a);
}
inline void make_monic(poly& a) {
 const u32 il= inv(a.back());
 for(auto& x: a) x= mul(x, il);
}
inline poly gcd(poly a, poly b) {
 trim(a), trim(b);
 while(!b.empty()) {
  make_monic(b);
  rem_monic(a, b);
  std::swap(a, b);
 }
 if(!a.empty()) make_monic(a);
 return a;
}
// g / d (d は g を割り切る monic)
inline poly divide(const poly& g, const poly& d) {
 poly a= g;
 const int dd= int(d.size()) - 1, dq= int(g.size()) - 1 - dd;
 poly q(dq + 1);
 for(int i= int(a.size()) - 1; i >= dd; --i) {
  const u32 c= a[i];
  q[i - dd]= c;
  if(!c) continue;
  for(int j= 0; j < dd; ++j) a[i - dd + j]= sub(a[i - dd + j], mul(c, d[j]));
 }
 return q;
}
inline void split(const poly& g, std::vector<u32>& roots, std::mt19937& rng) {
 const int dg= int(g.size()) - 1;
 if(dg <= 0) return;
 if(dg == 1) {
  roots.push_back(g[0] ? M - g[0] : 0);  // x + g0 (monic)
  return;
 }
 const ModF mf(g);
 while(true) {
  const u32 s= rng() % M;
  poly h= mf.pow_linear(s, (M - 1) / 2);
  if(h.empty()) h.push_back(0);
  h[0]= sub(h[0], 1), trim(h);
  poly d= gcd(g, h);
  const int dd= int(d.size()) - 1;
  if(0 < dd && dd < dg) {
   split(d, roots, rng);
   split(divide(g, d), roots, rng);
   return;
  }
 }
}
}  // namespace rf1
inline vector<u32> run(const vector<u32>& f_) {
 using namespace rf1;
 poly f(f_.begin(), f_.end());
 trim(f);
 vector<u32> roots;
 if(f.size() <= 1) return roots;
 int z= 0;
 while(f[z] == 0) ++z;
 if(z) roots.push_back(0), f.erase(f.begin(), f.begin() + z);
 if(f.size() <= 1) return roots;
 make_monic(f);
 std::mt19937 rng(20261118);
 const ModF mf(f);
 const poly h= mf.pow_linear(0, (M - 1) / 2);  // x^{(p-1)/2} mod f
 poly hm= h, hp= h;
 if(hm.empty()) hm.push_back(0);
 if(hp.empty()) hp.push_back(0);
 hm[0]= sub(hm[0], 1), hp[0]= add(hp[0], 1);
 trim(hm), trim(hp);
 split(gcd(f, hm), roots, rng);
 split(gcd(f, hp), roots, rng);
 return roots;
}
