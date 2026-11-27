#pragma once
// 任意の法の畳み込みの核 (実験版)。複素数の FFT。
// 係数を B bit ずつ P 桁に分け (a = Σ a_t 2^{B t})、桁どうしの畳み込み d_s = Σ_{t+u=s} conv(a_t, b_u) を作って、
// Σ d_s 2^{B s} を法 M で還元する。桁の列 [a_0, ..., a_{P-1}, b_0, ..., b_{P-1}] を 2 つずつ複素数の配列に詰めて
// 順変換 P 回、d の列を 2 つずつ詰めて逆変換 P 回で回す (a = b の 2 乗なら順変換は ceil(P/2) 回)。
// 桁の本数は、1 つの係数の上界 P min(n, m) 2^{2B} が 2^47 を超えないように選ぶ。超えるなら B を 15 から 10 に落とす。
// 倍精度の丸めの誤差はこの上界に比例し、2^49 で 0.3 くらいなので、2^47 なら 0.08 くらいで済む。
// B = 10 なら M <= 2^30、min(n, m) <= 2^20 で P <= 3。M > 2^30 と min(n, m) > 2^20 は素朴な積 (naive.hpp) に回す。
// 口はほかの核と同じ convolve_range<ZMod<M>>(a, n, b, m, lo, hi, out)。
#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstdint>
#include <type_traits>
#include <vector>
#include "neo/base/core/algebra/ZMod.hpp"
#include "neo/base/core/convolve/naive.hpp"
#include "_shared/ntt/_kernel_base.hpp"

namespace cfft_internal {
using u32= unsigned;
using u64= unsigned long long;
using u128= unsigned __int128;

// 8 bit のビットの反転の表。
struct rev_tab_t {
 u32 v[256];
 constexpr rev_tab_t(): v() {
  for (u32 i= 0; i < 256; ++i) {
   u32 r= 0;
   for (int b= 0; b < 8; ++b) r|= ((i >> b) & 1u) << (7 - b);
   v[i]= r;
  }
 }
};
inline constexpr rev_tab_t rev_tab{};

// 下位 lg bit のビットの反転 (lg <= 24)。
inline int rev_bits(int x, int lg) {
 const u32 r= (rev_tab.v[u32(x) & 255] << 16) | (rev_tab.v[(u32(x) >> 8) & 255] << 8) | rev_tab.v[(u32(x) >> 16) & 255];
 return int(r >> (24 - lg));
}

// 回転因子の表。長さ n の変換の、段 len = 2、4、…、n のひねり w[j] = exp(-2 pi i j / len) を段ごとに続けて持つ。
// 段 len の表の先頭は len/2 - 1。
struct twiddle_table {
 int n= 0;
 std::vector<double> wr, wi;
};

inline twiddle_table& twiddles(int n) {
 static twiddle_table t;
 if (t.n != n) {
  t.wr.resize(n - 1), t.wi.resize(n - 1);
  constexpr double pi= 3.141592653589793238462643383279502884;
  for (int len= 2; len <= n; len<<= 1) {
   const int h= len >> 1;
   const double step= -2.0 * pi / len;
   for (int j= 0; j < h; ++j) t.wr[h - 1 + j]= std::cos(step * j), t.wi[h - 1 + j]= std::sin(step * j);
  }
  t.n= n;
 }
 return t;
}

// 順変換 (DIT)。入口はビットを反転した並び、出口は自然な並び。
inline void dft_bitrev_in(double* re, double* im, int n) {
 const twiddle_table& t= twiddles(n);
 for (int len= 2; len <= n; len<<= 1) {
  const int h= len >> 1;
  const double* wr= t.wr.data() + h - 1;
  const double* wi= t.wi.data() + h - 1;
  for (int s= 0; s < n; s+= len)
   for (int j= 0; j < h; ++j) {
    const double cr= re[s + j + h], ci= im[s + j + h];
    const double xr= cr * wr[j] - ci * wi[j], xi= cr * wi[j] + ci * wr[j];
    const double ur= re[s + j], ui= im[s + j];
    re[s + j]= ur + xr, im[s + j]= ui + xi;
    re[s + j + h]= ur - xr, im[s + j + h]= ui - xi;
   }
 }
}

// 逆変換 (DIF、1/n は掛けない)。入口は自然な並び、出口はビットを反転した並びで n 倍の値。
inline void idft_bitrev_out(double* re, double* im, int n) {
 const twiddle_table& t= twiddles(n);
 for (int len= n; len >= 2; len>>= 1) {
  const int h= len >> 1;
  const double* wr= t.wr.data() + h - 1;
  const double* wi= t.wi.data() + h - 1;
  for (int s= 0; s < n; s+= len)
   for (int j= 0; j < h; ++j) {
    const double ur= re[s + j], ui= im[s + j];
    const double vr= re[s + j + h], vi= im[s + j + h];
    re[s + j]= ur + vr, im[s + j]= ui + vi;
    const double dr= ur - vr, di= ui - vi;
    re[s + j + h]= dr * wr[j] + di * wi[j];  // (dr + i di) * conj(w)
    im[s + j + h]= di * wr[j] - dr * wi[j];
   }
 }
}

// x < 2^64 の mod m (m < 2^30)。mu = floor(2^64 / m)。q は floor(x / m) か 1 小さい値なので、1 回引けば m 未満になる。
inline u64 reduce_small(u64 x, u64 m, u64 mu) {
 const u64 q= u64((u128(x) * mu) >> 64);
 u64 r= x - q * m;
 return r >= m ? r - m : r;
}

// P 桁ずつの畳み込み。桁は B bit ずつで、a、b の桁を [a_0, …, a_{P-1}, b_0, …, b_{P-1}] の順に 2 つずつ詰める。
// a = b の 2 乗のときは a の桁だけを詰める。d_s は配列 j の実部 (s = 2j) と虚部 (s = 2j + 1) に詰め直す。
template <int P> struct runner {
 template <class K> static void go(const K* a, int n, const K* b, int m, int lo, int hi, K* out, unsigned B, int L, int lg) {
  const u32 M= K::mod(), maski= (1u << B) - 1;
  const bool sq= a == b && n == m;
  const int ndig= sq ? P : 2 * P, narr= (ndig + 1) / 2;
  const double invL= 1.0 / L;
  const u64 mu= u64((u128(1) << 64) / M);
  u64 w[2 * P];  // 2^{B s} mod M
  w[0]= 1;
  for (int s= 1; s < 2 * P - 1; ++s) w[s]= w[s - 1] * ((u64(1) << B) % M) % M;
  static std::vector<double> rr[P], ii[P];
  for (int j= 0; j < P; ++j) rr[j].assign(L, 0.0), ii[j].assign(L, 0.0);
  for (int i= 0; i < n; ++i) {
   const int r= rev_bits(i, lg);
   const u32 v= a[i].val();
   for (int t= 0; t < P; ++t) {
    const double x= double((v >> (B * t)) & maski);
    if (t & 1) ii[t >> 1][r]= x; else rr[t >> 1][r]= x;
   }
  }
  if (!sq)
   for (int i= 0; i < m; ++i) {
    const int r= rev_bits(i, lg);
    const u32 v= b[i].val();
    for (int t= 0; t < P; ++t) {
     const double x= double((v >> (B * t)) & maski);
     const int g= P + t;
     if (g & 1) ii[g >> 1][r]= x; else rr[g >> 1][r]= x;
    }
   }
  for (int j= 0; j < narr; ++j) dft_bitrev_in(rr[j].data(), ii[j].data(), L);
  // 点ごとの積。各 k で 2P 個のスペクトルを分離し、d_s を作って詰め直す。
  const int ns= 2 * P - 1;
  for (int k= 0; k <= L / 2; ++k) {
   const int k2= (L - k) & (L - 1);
   double xr[2 * P], xi[2 * P];
   for (int j= 0; j < narr; ++j) {
    const double ar= rr[j][k], ai= ii[j][k], br= rr[j][k2], bi= ii[j][k2];
    xr[2 * j]= (ar + br) * 0.5, xi[2 * j]= (ai - bi) * 0.5;
    if (2 * j + 1 < ndig) xr[2 * j + 1]= (ai + bi) * 0.5, xi[2 * j + 1]= (br - ar) * 0.5;
   }
   const double* Br= sq ? xr : xr + P;
   const double* Bi= sq ? xi : xi + P;
   double dr[ns], di[ns];
   for (int s= 0; s < ns; ++s) {
    double sr= 0, si= 0;
    for (int t= 0; t < P; ++t) {
     const int u= s - t;
     if (u < 0 || u >= P) continue;
     sr+= xr[t] * Br[u] - xi[t] * Bi[u];
     si+= xr[t] * Bi[u] + xi[t] * Br[u];
    }
    dr[s]= sr, di[s]= si;
   }
   for (int j= 0; j < P; ++j) {
    const int s0= 2 * j, s1= 2 * j + 1;
    const double e1= s1 < ns ? di[s1] : 0.0, f1= s1 < ns ? dr[s1] : 0.0;
    rr[j][k]= dr[s0] - e1, ii[j][k]= di[s0] + f1;
    if (k2 != k) rr[j][k2]= dr[s0] + e1, ii[j][k2]= f1 - di[s0];
   }
  }
  for (int j= 0; j < P; ++j) idft_bitrev_out(rr[j].data(), ii[j].data(), L);
  for (int k= lo; k < hi; ++k) {
   const int r= rev_bits(k, lg);
   u64 acc= 0;
   for (int s= 0; s < ns; ++s) {
    const double val= ((s & 1) ? ii[s >> 1][r] : rr[s >> 1][r]) * invL;
    acc= reduce_small(acc + reduce_small(u64(std::llround(val)), M, mu) * w[s], M, mu);
   }
   out[k - lo]= K::raw(u32(acc));
  }
 }
};
}  // namespace cfft_internal

struct Cfft {
 // out[k - lo] = Σ_{i+j=k} a[i] b[j] (lo <= k < hi)。n >= 1、m >= 1、0 <= lo < hi <= n + m - 1。
 template <class K> static void convolve_range(const K* a, int n, const K* b, int m, int lo, int hi, K* out) {
  static_assert(is_zmod_v<K>, "Cfft は ZMod の係数だけ");
  using namespace cfft_internal;
  const u32 M= K::mod();
  if (M == 1) {
   std::fill(out, out + (hi - lo), K());
   return;
  }
  const int mn= std::min(n, m);
  if (mn < 60 || M > (1u << 30) || mn > (1 << 20)) {
   Naive::convolve_range(a, n, b, m, lo, hi, out);
   return;
  }
  const int sz= n + m - 1;
  const int need= std::max(hi, sz - lo);  // 巡回の長さ (折り返した項が [lo, hi) に入らない)
  int L= 1;
  while (L < need) L<<= 1;
  const int lg= __builtin_ctz(L);
  const int bits= 32 - __builtin_clz(M - 1);
  unsigned B= 15;
  int P= (bits + 14) / 15;
  if (u64(P) * mn << (2 * B) > (u64(1) << 47)) B= 10, P= (bits + 9) / 10;
  if (P == 1) runner<1>::go(a, n, b, m, lo, hi, out, B, L, lg);
  else if (P == 2) runner<2>::go(a, n, b, m, lo, hi, out, B, L, lg);
  else runner<3>::go(a, n, b, m, lo, hi, out, B, L, lg);
 }
};
