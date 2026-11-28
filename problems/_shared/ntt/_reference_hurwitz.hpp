#pragma once
// 参照実装 (二項畳み込み)。ライブラリを読まない。c[k] = Σ_i C(k, i) a[i] b[k-i] (mod 法)。
// 法の最小の素因数が k の範囲 (n + m - 1) 以上なら、階乗 1/k! で分けて普通の畳み込み 1 回で求める。
// そうでなければ EntropyIncreaser「任意模数二项卷积」の方法を、法の素数べきごとに分けて使い、
// 最後に法の下で CRT する (ライブラリの実装とは別の組み立てにして、突き合わせの意味を持たせる)。
// 素数べき p^e の下では、k! = u_k p^{v_p(k!)} と分け (u_k は p と互いに素)、
//   c_n = u_n Σ_k p^{d_k} (a_k u_k^{-1}) (b_{n-k} u_{n-k}^{-1}),  d_k = v_p(n!) - v_p(k!) - v_p((n-k)!)
// の和 S_n を 4 本の NTT の素数で厳密に戻す。S_n < (n+1) n (p^e - 1)^2 <= (n+m-1)^2 p^{2e} < 2^116。
#include <algorithm>
#include <vector>
#include "_shared/ntt/_reference.hpp"

namespace hurwitz_reference {

using u32= unsigned int;
using u64= unsigned long long;
using u128= __uint128_t;

// 4 本の NTT の素数。積は 2^115 より大きい。
constexpr u32 Q1= 998244353, Q2= 1004535809, Q3= 469762049, Q4= 167772161;
static_assert(u128(Q1) * Q2 * Q3 * Q4 > (u128(1) << 115), "4 本の素数の積が足りない");

inline u32 pow_mod(u32 a, u64 e, u32 p) {
 u32 r= 1;
 while (e) {
  if (e & 1) r= u32(u64(r) * a % p);
  a= u32(u64(a) * a % p), e>>= 1;
 }
 return r;
}
inline u32 inv_mod(u32 a, u32 p) { return pow_mod(a, p - 2, p); }  // p が素数のときだけ
// gcd(a, m) = 1 のときの a^{-1} mod m (法が素数でなくてもよい)。
inline u32 inv_mod_general(u32 a, u32 m) {
 long long r0= m, r1= a, s0= 0, s1= 1;
 while (r1) {
  const long long q= r0 / r1, r2= r0 - q * r1, s2= s0 - q * s1;
  r0= r1, r1= r2, s0= s1, s1= s2;
 }
 s0%= (long long)m;
 return u32(s0 < 0 ? s0 + m : s0);
}

// Garner の定数 (x = c0 + c1 Q1 + c2 Q1Q2 + c3 Q1Q2Q3)。
inline const u32 IQ1_2= inv_mod(u32(Q1 % Q2), Q2);
inline const u32 Q1_3= u32(Q1 % Q3);
inline const u32 IQ12_3= inv_mod(u32(u128(Q1) * Q2 % Q3), Q3);
inline const u32 Q1_4= u32(Q1 % Q4), Q12_4= u32(u128(Q1) * Q2 % Q4);
inline const u32 IQ123_4= inv_mod(u32(u128(u128(Q1) * Q2 % Q4) * Q3 % Q4), Q4);

// 4 本の剰余 (x < Q1Q2Q3Q4) から x mod 法。
inline u32 garner4(u32 r0, u32 r1, u32 r2, u32 r3, u32 mod) {
 const u32 c1= u32(u64((r1 + Q2 - r0 % Q2) % Q2) * IQ1_2 % Q2);
 const u32 x2= u32((u64(r0 % Q3) + u64(Q1_3) * c1) % Q3);
 const u32 c2= u32(u64((r2 + Q3 - x2) % Q3) * IQ12_3 % Q3);
 const u32 x3= u32((u64(r0 % Q4) + u64(Q1_4) * c1 % Q4 + u64(Q12_4) * c2 % Q4) % Q4);
 const u32 c3= u32(u64((r3 + Q4 - x3) % Q4) * IQ123_4 % Q4);
 const u128 x= u128(r0) + u128(Q1) * c1 + u128(Q1) * Q2 * c2 + u128(Q1) * Q2 * Q3 * c3;
 return u32(x % mod);
}

// 普通の畳み込み (mod 法)。値の上界 min(n, m) (mod - 1)^2 が 4 本の素数の積未満であること。
inline void conv4(const u32* a, int n, const u32* b, int m, u32* c, u32 mod) {
 const int sz= n + m - 1;
 const u32 qs[4]= {Q1, Q2, Q3, Q4};
 std::vector<u32> xa(n), xb(m);
 std::vector<u32> r[4];
 for (int t= 0; t < 4; ++t) r[t].resize(sz);
 for (int t= 0; t < 4; ++t) {
  const u32 q= qs[t];
  for (int i= 0; i < n; ++i) xa[i]= a[i] % q;
  for (int i= 0; i < m; ++i) xb[i]= b[i] % q;
  if (t == 0) ntt_reference::convolve_range<Q1>(xa.data(), n, xb.data(), m, 0, sz, r[0].data());
  else if (t == 1) ntt_reference::convolve_range<Q2>(xa.data(), n, xb.data(), m, 0, sz, r[1].data());
  else if (t == 2) ntt_reference::convolve_range<Q3>(xa.data(), n, xb.data(), m, 0, sz, r[2].data());
  else ntt_reference::convolve_range<Q4>(xa.data(), n, xb.data(), m, 0, sz, r[3].data());
 }
 for (int k= 0; k < sz; ++k) c[k]= garner4(r[0][k], r[1][k], r[2][k], r[3][k], mod);
}

// 素朴な二項畳み込み (O(n m))。二項係数はパスカルの漸化式で作る。
inline void naive_binom(const u32* a, int n, const u32* b, int m, u32* c, u32 mod) {
 const int sz= n + m - 1;
 std::vector<u32> bin(std::min(n, m), 0);
 bin[0]= 1;
 for (int k= 0; k < sz; ++k) {
  u64 acc= 0;
  if (n <= m) {
   for (int i= std::max(0, k - m + 1), e= std::min(n - 1, k); i <= e; ++i)
    acc+= u64(a[i]) * b[k - i] % mod * bin[i] % mod;
  } else {
   for (int i= std::max(0, k - m + 1), e= std::min(n - 1, k); i <= e; ++i)
    acc+= u64(a[i]) * b[k - i] % mod * bin[k - i] % mod;
  }
  c[k]= u32(acc % mod);
  for (int i= std::min(int(bin.size()) - 1, k + 1); i > 0; --i) bin[i]= u32((u64(bin[i]) + bin[i - 1]) % mod);
 }
}

// k! の p を抜いた部分 u_k (mod 法) と、その逆元。strip[k] = k / p^{v_p(k)}。
inline void unit_factorials(int sz, u32 p, u32 mod, std::vector<u32>& u, std::vector<u32>& iu, std::vector<u32>& strip) {
 u.assign(sz, 1), iu.assign(sz, 1), strip.assign(sz, 1);
 for (int k= 1; k < sz; ++k) {
  u32 x= u32(k);
  while (x % p == 0) x/= p;
  strip[k]= x;
  u[k]= u32(u64(u[k - 1]) * x % mod);
 }
 iu[sz - 1]= inv_mod_general(u[sz - 1], mod);
 for (int k= sz - 1; k > 0; --k) iu[k - 1]= u32(u64(iu[k]) * strip[k] % mod);  // u[k] = u[k-1] strip[k]
}

// 素数べき pe の下での二項畳み込み。
inline void binom_mod_prime_power(const u32* a, int n, const u32* b, int m, u32* c, u32 pe, u32 p) {
 const int sz= n + m - 1;
 std::vector<u32> u, iu, strip;
 unit_factorials(sz, p, pe, u, iu, strip);
 std::vector<u32> r[4];
 for (int t= 0; t < 4; ++t) r[t].assign(sz, 0);
 const u32 qs[4]= {Q1, Q2, Q3, Q4};
 std::vector<u32> xa(n), xb(m), cv(sz);
 for (int t= 0; t < 4; ++t) {
  const u32 q= qs[t];
  // P_k = p^{-v_p(k!)} mod q、Q_k = p^{v_p(k!)} mod q。
  const u32 ip= inv_mod(p % q, q);
  u32 pk= 1;
  for (int k= 0; k < sz; ++k) {
   if (k > 0)
    for (u32 x= u32(k); x % p == 0; x/= p) pk= u32(u64(pk) * ip % q);
   if (k < n) xa[k]= u32(u64(u32(u64(a[k]) * iu[k] % pe) % q) * pk % q);
   if (k < m) xb[k]= u32(u64(u32(u64(b[k]) * iu[k] % pe) % q) * pk % q);
  }
  if (t == 0) ntt_reference::convolve_range<Q1>(xa.data(), n, xb.data(), m, 0, sz, cv.data());
  else if (t == 1) ntt_reference::convolve_range<Q2>(xa.data(), n, xb.data(), m, 0, sz, cv.data());
  else if (t == 2) ntt_reference::convolve_range<Q3>(xa.data(), n, xb.data(), m, 0, sz, cv.data());
  else ntt_reference::convolve_range<Q4>(xa.data(), n, xb.data(), m, 0, sz, cv.data());
  // Q_k = p^{v_p(k!)} を掛ける。
  u32 qk= 1;
  for (int k= 0; k < sz; ++k) {
   if (k > 0)
    for (u32 x= u32(k); x % p == 0; x/= p) qk= u32(u64(qk) * p % q);
   r[t][k]= u32(u64(cv[k]) * qk % q);
  }
 }
 for (int k= 0; k < sz; ++k) {
  const u32 s= garner4(r[0][k], r[1][k], r[2][k], r[3][k], pe);
  c[k]= u32(u64(u[k]) * s % pe);
 }
}

// 二項畳み込み c[k] = Σ_i C(k, i) a[i] b[k-i] (mod 法)。a、b の値は法未満。
inline void convolve(const u32* a, int n, const u32* b, int m, u32* c, u32 mod) {
 const int sz= n + m - 1;
 if (mod == 1) {
  std::fill(c, c + sz, 0u);
  return;
 }
 if (1LL * n * m <= 4096) {
  naive_binom(a, n, b, m, c, mod);
  return;
 }
 // 素因数分解。
 u32 ps[10];
 int np= 0;
 u32 x= mod;
 for (u32 p= 2; u64(p) * p <= x; ++p) {
  if (x % p) continue;
  ps[np++]= p;
  while (x % p == 0) x/= p;
 }
 if (x > 1) ps[np++]= x;
 const u128 bound= u128(std::min(n, m)) * u128(mod - 1) * (mod - 1);
 const u128 cap= u128(Q1) * Q2 * Q3 * Q4;
 if (ps[0] >= u32(sz)) {
  // 階乗で分ける道。1, ..., sz-1 は法で割り切れない。
  if (bound >= cap) {
   naive_binom(a, n, b, m, c, mod);
   return;
  }
  std::vector<u32> fact(sz), ifact(sz), fa(n), fb(m), fc(sz);
  fact[0]= 1;
  for (int i= 0; i < sz - 1; ++i) fact[i + 1]= u32(u64(fact[i]) * (i + 1) % mod);
  ifact[sz - 1]= inv_mod_general(fact[sz - 1], mod);
  for (int i= sz - 1; i > 0; --i) ifact[i - 1]= u32(u64(ifact[i]) * i % mod);
  for (int i= 0; i < n; ++i) fa[i]= u32(u64(a[i]) * ifact[i] % mod);
  for (int i= 0; i < m; ++i) fb[i]= u32(u64(b[i]) * ifact[i] % mod);
  conv4(fa.data(), n, fb.data(), m, fc.data(), mod);
  for (int k= 0; k < sz; ++k) c[k]= u32(u64(fc[k]) * fact[k] % mod);
  return;
 }
 // 素数べきごとの道。上界 (n+1) n (pe-1)^2 <= sz^2 mod^2 が 4 本の素数の積未満であること。
 const u128 bound2= u128(sz) * sz * u128(mod) * mod;
 if (bound2 >= cap) {
  naive_binom(a, n, b, m, c, mod);
  return;
 }
 std::vector<u32> cur(sz, 0);
 u64 cur_mod= 1;
 u32 x2= mod, pe_list[10];
 int npe= 0;
 for (u32 p= 2; u64(p) * p <= x2; ++p) {
  if (x2 % p) continue;
  u32 pe= 1;
  while (x2 % p == 0) x2/= p, pe*= p;
  pe_list[npe++]= pe;
 }
 if (x2 > 1) pe_list[npe++]= x2;
 std::vector<u32> cj(sz);
 for (int t= 0; t < npe; ++t) {
  const u32 pe= pe_list[t], p= ps[t];
  binom_mod_prime_power(a, n, b, m, cj.data(), pe, p);
  // x <- x + cur_mod ((c_j - x) cur_mod^{-1} mod pe)
  const u32 icm= inv_mod_general(u32(cur_mod % pe), pe);
  for (int k= 0; k < sz; ++k) {
   const u32 t2= u32(u64((cj[k] + pe - cur[k] % pe) % pe) * icm % pe);
   cur[k]= u32((cur[k] + cur_mod * t2) % mod);
  }
  cur_mod*= pe;
 }
 std::copy(cur.begin(), cur.end(), c);
}

}  // namespace hurwitz_reference
