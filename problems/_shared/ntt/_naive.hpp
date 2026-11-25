#pragma once
// self/ntt の愚直解。参照実装とは別の考え方で書く。
// dft は定義どおりの Σ_j a[j] w^{jk} を自然な順で並べる。convolve_range は二重ループ。
#include <algorithm>
#include <vector>

namespace ntt_naive {

using u32= unsigned int;
using u64= unsigned long long;
using u128= __uint128_t;

inline u32 mul_mod(u32 a, u32 b, u32 mod) { return u32(u64(a) * b % mod); }

inline u32 pow_mod(u32 a, u64 e, u32 mod) {
 u32 r= 1;
 while (e) {
  if (e & 1) r= mul_mod(r, a, mod);
  a= mul_mod(a, a, mod), e>>= 1;
 }
 return r;
}

inline u32 primitive_root(u32 mod) {
 u32 n= mod - 1;
 std::vector<u32> ps;
 for (u32 p= 2; u64(p) * p <= n; ++p) {
  if (n % p) continue;
  ps.push_back(p);
  while (n % p == 0) n/= p;
 }
 if (n > 1) ps.push_back(n);
 for (u32 g= 2;; ++g) {
  bool ok= true;
  for (u32 p: ps)
   if (pow_mod(g, (mod - 1) / p, mod) == 1) {
    ok= false;
    break;
   }
  if (ok) return g;
 }
}

// out[k] = Σ_j a[j] w^{jk}。n は mod - 1 を割ること (2 の冪だけを使う)。
inline void dft(u32 *a, int n, u32 mod) {
 if (n <= 1) return;
 const u32 w= pow_mod(primitive_root(mod), (mod - 1) / n, mod);
 std::vector<u32> wj(n), out(n);
 wj[0]= 1;
 for (int j= 1; j < n; ++j) wj[j]= mul_mod(wj[j - 1], w, mod);
 for (int k= 0; k < n; ++k) {
  u128 s= 0;
  for (int j= 0; j < n; ++j) s+= (u128)a[j] * wj[u64(j) * k % n];
  out[k]= u32(s % mod);
 }
 std::copy(out.begin(), out.end(), a);
}

// out[k] = (1/n) Σ_j a[j] w^{-jk}。dft の逆。
inline void idft(u32 *a, int n, u32 mod) {
 if (n <= 1) return;
 const u32 w= pow_mod(primitive_root(mod), (mod - 1) - (mod - 1) / n, mod);
 std::vector<u32> wj(n), out(n);
 wj[0]= 1;
 for (int j= 1; j < n; ++j) wj[j]= mul_mod(wj[j - 1], w, mod);
 for (int k= 0; k < n; ++k) {
  u128 s= 0;
  for (int j= 0; j < n; ++j) s+= (u128)a[j] * wj[u64(j) * k % n];
  out[k]= u32(s % mod);
 }
 const u32 iv= pow_mod(u32(n), mod - 2, mod);
 for (int k= 0; k < n; ++k) a[k]= mul_mod(out[k], iv, mod);
}

// a[0..n) は長さ n の変換。係数に戻して 0 を詰め、長さ 2n の変換を作る。
inline void dft_doubling(u32 *a, int n, u32 mod) {
 std::vector<u32> c(a, a + n);
 idft(c.data(), n, mod);
 std::fill(a, a + 2 * n, 0u);
 std::copy(c.begin(), c.end(), a);
 dft(a, 2 * n, mod);
}

// c[k - lo] = Σ_{i+j=k} a[i] b[j] (lo ≤ k < hi)。
inline void convolve_range(const u32 *a, int n, const u32 *b, int m, int lo, int hi, u32 *c, u32 mod) {
 for (int k= lo; k < hi; ++k) {
  u128 s= 0;
  for (int i= std::max(0, k - m + 1), e= std::min(n - 1, k); i <= e; ++i) s+= (u128)a[i] * b[k - i];
  c[k - lo]= u32(s % mod);
 }
}

}  // namespace ntt_naive
