#pragma once
// 参照実装 (任意の法)。3 つの NTT の素数で _reference.hpp の NTT を回し、Garner の方法で法 mod の値に戻す。
// 小さい積は二重ループで求める。法は素数でも合成数でもよい (mod < 2^30)。
#include <algorithm>
#include <vector>
#include "_shared/ntt/_reference.hpp"

namespace ntt_reference_crt {

using u32= unsigned int;
using u64= unsigned long long;
using u128= __uint128_t;

constexpr u32 P1= 998244353, P2= 1004535809, P3= 469762049;
constexpr u64 P1P2= u64(P1) * P2;
static_assert(u128(P1) * P2 * P3 > (u128(1) << 88), "3 つの素数の積が足りない");

inline u32 pow_mod(u32 a, u64 e, u32 p) {
 u32 r= 1;
 while (e) {
  if (e & 1) r= u32(u64(r) * a % p);
  a= u32(u64(a) * a % p), e>>= 1;
 }
 return r;
}

// Garner の定数: g12 = P1^{-1} mod P2、g13 = (P1 P2)^{-1} mod P3。
inline const u32 G12= pow_mod(P1 % P2, u64(P2) - 2, P2);
inline const u32 G13= pow_mod(u32(P1P2 % P3), u64(P3) - 2, P3);
inline const u32 P1_3= P1 % P3;

inline u32 modp(u32 x, u32 p) {
 while (x >= p) x-= p;
 return x;
}

// c[k - lo] = Σ_{i+j=k} a[i] b[j] (mod 法)。a と b の値は法未満。
inline void convolve_range(const u32 *a, int n, const u32 *b, int m, int lo, int hi, u32 *c, u32 mod) {
 const int sz= n + m - 1;
 if (mod == 1) {
  std::fill(c, c + (hi - lo), 0u);
  return;
 }
 // 3 つの素数の積に収まらない大きさは二重ループに回す (この問題の範囲では起きない)。
 if (1LL * n * m <= 4096 || u128(std::min(n, m)) * u128(mod - 1) * (mod - 1) >= u128(P1) * P2 * P3) {
  for (int k= lo; k < hi; ++k) {
   u128 s= 0;
   for (int i= std::max(0, k - m + 1), e= std::min(n - 1, k); i <= e; ++i) s+= u128(a[i]) * b[k - i];
   c[k - lo]= u32(s % mod);
  }
  return;
 }
 // 係数の和の上界 min(n, m) (mod - 1)^2 が収まる素数の本数だけ回す。
 const u128 bound= u128(std::min(n, m)) * u128(mod - 1) * (mod - 1);
 const int cnt= bound < u128(P1) ? 1 : bound < u128(P1) * P2 ? 2 : 3;
 std::vector<u32> xa(n), xb(m), r1(sz), r2(cnt >= 2 ? sz : 0), r3(cnt >= 3 ? sz : 0);
 const u32 ps[3]= {P1, P2, P3};
 for (int t= 0; t < cnt; ++t) {
  const u32 p= ps[t];
  for (int i= 0; i < n; ++i) xa[i]= modp(a[i], p);
  for (int i= 0; i < m; ++i) xb[i]= modp(b[i], p);
  if (t == 0) ntt_reference::convolve_range<P1>(xa.data(), n, xb.data(), m, 0, sz, r1.data());
  else if (t == 1) ntt_reference::convolve_range<P2>(xa.data(), n, xb.data(), m, 0, sz, r2.data());
  else ntt_reference::convolve_range<P3>(xa.data(), n, xb.data(), m, 0, sz, r3.data());
 }
 if (cnt == 1) {
  for (int k= lo; k < hi; ++k) c[k - lo]= r1[k] % mod;
  return;
 }
 if (cnt == 2) {
  for (int k= lo; k < hi; ++k) {
   const u32 y1= r1[k];
   const u32 d2= u32((u64(r2[k]) + P2 - y1) % P2);
   const u32 t1= u32(u64(d2) * G12 % P2);
   c[k - lo]= u32((u64(y1) + u64(P1) * t1) % mod);
  }
  return;
 }
 for (int k= lo; k < hi; ++k) {
  const u32 y1= r1[k];
  const u32 d2= u32((u64(r2[k]) + P2 - y1) % P2);
  const u32 t1= u32(u64(d2) * G12 % P2);
  const u32 u= u32((u64(y1 % P3) + u64(P1_3) * t1) % P3);
  const u32 d3= u32((u64(r3[k]) + P3 - u) % P3);
  const u32 t2= u32(u64(d3) * G13 % P3);
  const u128 x= u128(y1) + u128(P1) * t1 + u128(P1P2) * t2;
  c[k - lo]= u32(x % mod);
 }
}

}  // namespace ntt_reference_crt
