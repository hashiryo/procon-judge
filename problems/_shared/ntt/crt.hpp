#pragma once
// 任意の法 (素数でも合成数でもよい) の畳み込みの核 (実験版)。3 つの NTT の素数で単独の NTT の核 (2 本目、スカラー基数 4) を回し、
// Garner の方法で法 M の値に戻す。係数の和の上界は min(n, m) (M - 1)^2 で、M < 2^30、n, m <= 2^28 なら 2^86 なので、
// 3 つの素数の積 (2^88 より大きい) で足りる。
// 口はほかの核と同じ convolve_range<ZMod<M>>(a, n, b, m, lo, hi, out)。K は ZMod (M は素数でも合成数でもよい)。
// 小さい積は素朴な積 (naive.hpp) に回す。
#include <algorithm>
#include <cassert>
#include <cstdint>
#include <type_traits>
#include <vector>
#include "neo/base/core/algebra/ZMod.hpp"
#include "neo/base/core/convolve/naive.hpp"
#include "_shared/ntt/_kernel_base.hpp"
#include "_shared/ntt/ntt_scalar_r4.hpp"

namespace crt_internal {
using u32= unsigned;
using u64= unsigned long long;
using u128= unsigned __int128;

// 3 つの NTT の素数。積は 998244353 * 1004535809 * 469762049 ≈ 4.71e26 > 2^88。
constexpr u32 P1= 998244353, P2= 1004535809, P3= 469762049;
static_assert(ntt_friendly<P1> && ntt_friendly<P2> && ntt_friendly<P3>);
constexpr u64 P1P2= u64(P1) * P2;

constexpr u32 pow_mod_c(u32 a, u64 e, u32 p) {
 u64 r= 1, b= a;
 for (; e; e>>= 1, b= b * b % p)
  if (e & 1) r= r * b % p;
 return u32(r);
}
// Garner の定数: g12 = P1^{-1} mod P2、g13 = (P1 P2)^{-1} mod P3、p1_3 = P1 mod P3。
constexpr u32 G12= pow_mod_c(P1 % P2, u64(P2) - 2, P2);
constexpr u32 G13= pow_mod_c(u32(P1P2 % P3), u64(P3) - 2, P3);
constexpr u32 P1_3= P1 % P3;

// 0 <= x < 2^30 の、素数 p (469762049 以上) での剰余。p = P3 では 2 回引くことがある。
inline u32 modp(u32 x, u32 p) {
 if (x >= p) x-= p;
 if (x >= p) x-= p;
 return x;
}

// x < 2^64 の mod m (m < 2^30)。mu = floor(2^64 / m) の Barrett を 2 段。
// 1 段目で r < m + x m / 2^64 < 2m、2 段目で r < m + 1 になるので、最後に 1 回引けばよい。
struct barrett {
 u64 m, mu, r64;  // r64 = 2^64 mod m
 explicit barrett(u32 mod): m(mod), mu(mod ? ~0ull / mod : 0), r64(mod ? u64((u128(1) << 64) % mod) : 0) {}
 u32 reduce(u64 x) const {
  u64 q= u64((u128(x) * mu) >> 64);
  u64 r= x - q * m;
  q= u64((u128(r) * mu) >> 64);
  r= r - q * m;
  return u32(r >= m ? r - m : r);
 }
 u32 reduce128(u128 x) const {  // x < 2^89
  const u32 hi= reduce(u64(x >> 64));
  return reduce(u64(hi) * r64 + reduce(u64(x)));
 }
};

// 素数 P での畳み込み (単独の NTT の核 Ntt をそのまま使う)。out[0..sz) に全体の積。
template <class Ntt, u32 P> void prime_conv(const u32* a, int n, const u32* b, int m, int sz, bool sq, u32* out) {
 using Z= ZMod<P>;
 std::vector<Z> fa(n), fb(sq ? 0 : m), fc(sz);
 for (int i= 0; i < n; ++i) fa[i]= Z::raw(a[i]);
 if (!sq)
  for (int i= 0; i < m; ++i) fb[i]= Z::raw(b[i]);
 Ntt::template convolve_range<Z>(fa.data(), n, sq ? fa.data() : fb.data(), m, 0, sz, fc.data());
 for (int i= 0; i < sz; ++i) out[i]= fc[i].val();
}
}  // namespace crt_internal

// 3 素数の NTT を核 Ntt で回す CRT。Ntt は単独の NTT の核 (convolve_range<K> を持つ) なら何でもよい。
template <class Ntt> struct crt_kernel {
 // out[k - lo] = Σ_{i+j=k} a[i] b[j] (lo <= k < hi)。n >= 1、m >= 1、0 <= lo < hi <= n + m - 1。
 template <class K> static void convolve_range(const K* a, int n, const K* b, int m, int lo, int hi, K* out) {
  static_assert(is_zmod_v<K>, "Crt は ZMod の係数だけ");
  using namespace crt_internal;
  const u32 M= K::mod();
  if (M == 1) {
   std::fill(out, out + (hi - lo), K());
   return;
  }
  // 小さい積と、係数の和が 3 素数の積を超えうる大きさは Naive に回す (この問題の範囲では起きない)。
  if (std::min(n, m) < 60 || u128(std::min(n, m)) * u128(M - 1) * (M - 1) >= u128(P1) * P2 * P3) {
   Naive::convolve_range(a, n, b, m, lo, hi, out);
   return;
  }
  const int sz= n + m - 1;
  const bool sq= a == b && n == m;
  // 係数の和の上界 min(n, m) (M - 1)^2 が収まる素数の本数だけ回す (M が小さいと 1 本か 2 本で済む)。
  const u128 bound= u128(std::min(n, m)) * u128(M - 1) * (M - 1);
  const int cnt= bound < u128(P1) ? 1 : bound < u128(P1) * P2 ? 2 : 3;
  std::vector<u32> xa(n), xb(sq ? 0 : m);
  std::vector<u32> r1(sz), r2(cnt >= 2 ? sz : 0), r3(cnt >= 3 ? sz : 0);
  const u32 ps[3]= {P1, P2, P3};
  const barrett bm(M);
  for (int t= 0; t < cnt; ++t) {
   const u32 p= ps[t];
   for (int i= 0; i < n; ++i) xa[i]= modp(a[i].val(), p);
   if (!sq)
    for (int i= 0; i < m; ++i) xb[i]= modp(b[i].val(), p);
   u32* r= t == 0 ? r1.data() : t == 1 ? r2.data() : r3.data();
   if (t == 0) crt_internal::prime_conv<Ntt, P1>(xa.data(), n, xb.data(), m, sz, sq, r);
   else if (t == 1) crt_internal::prime_conv<Ntt, P2>(xa.data(), n, xb.data(), m, sz, sq, r);
   else crt_internal::prime_conv<Ntt, P3>(xa.data(), n, xb.data(), m, sz, sq, r);
  }
  if (cnt == 1) {
   for (int k= lo; k < hi; ++k) out[k - lo]= K::raw(bm.reduce(r1[k]));
   return;
  }
  if (cnt == 2) {
   for (int k= lo; k < hi; ++k) {
    const u32 y1= r1[k];
    const u32 d2= u32((u64(r2[k]) + P2 - y1) % P2);
    const u32 t1= u32((u64(d2) * G12) % P2);
    out[k - lo]= K::raw(bm.reduce(u64(y1) + u64(P1) * t1));
   }
   return;
  }
  // Garner: x = x1 + P1 t1 + P1 P2 t2 (mod M)。
  for (int k= lo; k < hi; ++k) {
   const u32 y1= r1[k];
   const u32 d2= u32((u64(r2[k]) + P2 - y1) % P2);  // y1 < P1 < P2
   const u32 t1= u32((u64(d2) * G12) % P2);
   const u32 u= u32((u64(y1 % P3) + u64(P1_3) * t1) % P3);
   const u32 d3= u32((u64(r3[k]) + P3 - u) % P3);
   const u32 t2= u32((u64(d3) * G13) % P3);
   const u128 x= u128(y1) + u128(P1) * t1 + u128(P1P2) * t2;
   out[k - lo]= K::raw(bm.reduce128(x));
  }
 }
};
using Crt= crt_kernel<NttR4>;
