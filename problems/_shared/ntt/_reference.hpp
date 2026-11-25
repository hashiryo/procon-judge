#pragma once
// self/ntt の参照実装。ライブラリを読まない。
// dft は今の Library の NTT と同じ並び (DIF のまま並べ替えをせず、評価点が index の bit 反転の順に入る)。
// idft はその逆で、1/n まで掛ける。dft_doubling は先頭 n 個の変換から長さ 2n の変換を作る。
// convolve_range は全体の積を NTT で求めて [lo, hi) を切り出す。小さい積は二重ループで求める。
// 意味は algo-notes の notes/ntt-fps.md の「核が出す関数」と「素の変換」に合わせてある。
#include <algorithm>
#include <vector>

namespace ntt_reference {

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

// mod は素数。mod - 1 の素因数を集め、どの p でも g^((mod-1)/p) != 1 となる最小の g を返す。
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

template <u32 MOD> const u32 &root() {
 static const u32 g= primitive_root(MOD);
 return g;
}

// DIF。a[i] に Σ_j a[j] w^{j bitrev(i)} が入る。
template <u32 MOD> void dft(u32 *a, int n) {
 if (n <= 1) return;
 const u32 g= root<MOD>();
 std::vector<u32> wt(n>> 1);
 for (int len= n; len >= 2; len>>= 1) {
  const int h= len>> 1;
  const u32 wlen= pow_mod(g, u64(MOD - 1) / u64(len), MOD);
  wt[0]= 1;
  for (int j= 1; j < h; ++j) wt[j]= mul_mod(wt[j - 1], wlen, MOD);
  for (int i= 0; i < n; i+= len) {
   u32 *x0= a + i, *x1= x0 + h;
   for (int j= 0; j < h; ++j) {
    const u32 u= x0[j], v= x1[j], s= u+v;
    x0[j]= s >= MOD ? s - MOD : s;
    x1[j]= mul_mod(u >= v ? u - v : u + MOD - v, wt[j], MOD);
   }
  }
 }
}

// DIT。bit 反転の順の入力を、係数の順に戻して 1/n を掛ける。
template <u32 MOD> void idft(u32 *a, int n) {
 if (n <= 1) return;
 const u32 g= root<MOD>();
 std::vector<u32> wt(n>> 1);
 for (int len= 2; len <= n; len<<= 1) {
  const int h= len>> 1;
  const u32 wlen= pow_mod(g, u64(MOD - 1) - u64(MOD - 1) / u64(len), MOD);
  wt[0]= 1;
  for (int j= 1; j < h; ++j) wt[j]= mul_mod(wt[j - 1], wlen, MOD);
  for (int i= 0; i < n; i+= len) {
   u32 *x0= a + i, *x1= x0 + h;
   for (int j= 0; j < h; ++j) {
    const u32 u= x0[j], q= mul_mod(x1[j], wt[j], MOD), s= u + q;
    x0[j]= s >= MOD ? s - MOD : s;
    x1[j]= u >= q ? u - q : u + MOD - q;
   }
  }
 }
 const u32 iv= pow_mod(u32(n), MOD - 2, MOD);
 for (int i= 0; i < n; ++i) a[i]= mul_mod(a[i], iv, MOD);
}

// a[0..n) は長さ n の変換。係数に戻して 0 を詰め、長さ 2n の変換を a[0..2n) に作る。
template <u32 MOD> void dft_doubling(u32 *a, int n) {
 std::vector<u32> c(a, a + n);
 idft<MOD>(c.data(), n);
 std::fill(a, a + 2 * n, 0u);
 std::copy(c.begin(), c.end(), a);
 dft<MOD>(a, 2 * n);
}

// c[k - lo] = Σ_{i+j=k} a[i] b[j] (lo ≤ k < hi)。a と b が同じ配列なら 2 乗として求める。
template <u32 MOD> void convolve_range(const u32 *a, int n, const u32 *b, int m, int lo, int hi, u32 *c) {
 const int sz= n + m - 1;
 if (1LL * n * m <= 4096) {
  for (int k= lo; k < hi; ++k) {
   u128 s= 0;
   for (int i= std::max(0, k - m + 1), e= std::min(n - 1, k); i <= e; ++i) s+= (u128)a[i] * b[k - i];
   c[k - lo]= u32(s % MOD);
  }
  return;
 }
 int l= 1;
 while (l < sz) l<<= 1;
 std::vector<u32> fa(a, a + n), fb;
 fa.resize(l, 0);
 if (a == b && n == m) {
  dft<MOD>(fa.data(), l);
  for (int i= 0; i < l; ++i) fa[i]= mul_mod(fa[i], fa[i], MOD);
 } else {
  fb.assign(b, b + m), fb.resize(l, 0);
  dft<MOD>(fa.data(), l), dft<MOD>(fb.data(), l);
  for (int i= 0; i < l; ++i) fa[i]= mul_mod(fa[i], fb[i], MOD);
 }
 idft<MOD>(fa.data(), l);
 for (int k= lo; k < hi; ++k) c[k - lo]= fa[k];
}

}  // namespace ntt_reference
