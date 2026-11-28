#pragma once
// Burnikel–Ziegler の再帰的な割り算 (試作)。NeoLibrary の BigInt の桁 (10^9 進の u32、下位から) に対して、
// D2n1n / D3n2n の 2 段の再帰で割る。桁数が小さいか奇数のときは Knuth の D に落とす。
// bz_divmod(a, b, q, r) は正の大きさだけを扱う (符号は呼ぶ側)。
#include <algorithm>
#include <cassert>
#include <vector>
#include "neo/base/core/nt/BigInt.hpp"

namespace bigint_bz {
using bigint_internal::BASE;
using bigint_internal::cmp_abs;
using bigint_internal::trim;
using bigint_internal::u32;

inline std::vector<u32> low(const std::vector<u32>& a, int k) {  // a mod BASE^k
 if (int(a.size()) <= k) return a;
 return std::vector<u32>(a.begin(), a.begin() + k);
}
inline std::vector<u32> high(const std::vector<u32>& a, int k) {  // a / BASE^k
 if (int(a.size()) <= k) return {};
 return std::vector<u32>(a.begin() + k, a.end());
}
inline std::vector<u32> shift(const std::vector<u32>& a, int k) {  // a * BASE^k
 if (a.empty()) return {};
 std::vector<u32> r(k, 0);
 r.insert(r.end(), a.begin(), a.end());
 return r;
}
inline std::vector<u32> add(const std::vector<u32>& a, const std::vector<u32>& b) { return bigint_internal::add_abs(a, b); }
inline std::vector<u32> mul(const std::vector<u32>& a, const std::vector<u32>& b) { return bigint_internal::mul(a, b); }
inline bool geq(const std::vector<u32>& a, const std::vector<u32>& b) { return cmp_abs(a, b) >= 0; }
inline std::vector<u32> pow_base(int k) {  // BASE^k
 std::vector<u32> r(k + 1, 0);
 r[k]= 1;
 return r;
}
inline std::vector<u32> dec(const std::vector<u32>& a) {
 std::vector<u32> r= a;
 const std::vector<u32> one(1, 1);
 bigint_internal::sub_abs_inplace(r, one);
 return r;
}
inline std::vector<u32> inc(const std::vector<u32>& a) { return add(a, std::vector<u32>(1, 1)); }
inline std::vector<u32> diff(const std::vector<u32>& a, const std::vector<u32>& b) {  // a - b (a >= b)
 std::vector<u32> r= a;
 bigint_internal::sub_abs_inplace(r, b);
 return r;
}

inline void d3n2n(const std::vector<u32>& A, const std::vector<u32>& B, std::vector<u32>& Q, std::vector<u32>& R);

// A (2n 桁以下) / B (n 桁) の商と余り。A < B * BASE^n を仮定する。
inline void d2n1n(const std::vector<u32>& A, const std::vector<u32>& B, std::vector<u32>& Q, std::vector<u32>& R) {
 const int n= int(B.size());
 if (cmp_abs(A, B) < 0) {
  Q.clear(), R= A;
  return;
 }
 if (n <= 32 || (n & 1)) {
  bigint_internal::knuth_divmod(A.data(), int(A.size()), B.data(), n, Q, R);
  return;
 }
 const int n2= n / 2;
 const std::vector<u32> ahi= high(A, n2), alo= low(A, n2);
 std::vector<u32> q1, r1;
 d3n2n(ahi, B, q1, r1);
 std::vector<u32> a2= add(shift(r1, n2), alo);
 std::vector<u32> q0, r0;
 d3n2n(a2, B, q0, r0);
 Q= add(shift(q1, n2), q0);
 R= r0;
 trim(Q), trim(R);
}

// A (3n/2 桁以下) / B (n 桁) の商と余り。A < B * BASE^(n/2) を仮定する。
inline void d3n2n(const std::vector<u32>& A, const std::vector<u32>& B, std::vector<u32>& Q, std::vector<u32>& R) {
 const int n= int(B.size());
 if (cmp_abs(A, B) < 0) {
  Q.clear(), R= A;
  return;
 }
 if (n <= 32 || (n & 1) || int(A.size()) <= n) {
  bigint_internal::knuth_divmod(A.data(), int(A.size()), B.data(), n, Q, R);
  return;
 }
 const int n2= n / 2;
 const std::vector<u32> b1= high(B, n2), b0= low(B, n2);
 const std::vector<u32> a2= high(A, n), a1= low(high(A, n2), n2), a0= low(A, n2);
 std::vector<u32> Ah= add(shift(a2, n2), a1);
 std::vector<u32> Qh, Rh;
 d2n1n(Ah, b1, Qh, Rh);
 const std::vector<u32> beta= pow_base(n2);  // BASE^n2
 if (geq(Qh, beta)) {  // 商が大きすぎる: Qh = BASE^n2 - 1 に直し、Rh を作り直す
  Qh= dec(beta);
  Rh= diff(Ah, mul(Qh, b1));
 }
 std::vector<u32> X= add(shift(Rh, n2), a0);
 const std::vector<u32> Y= mul(Qh, b0);
 // R = X - Y。負なら B を足しながら Qh を下げる (補正は定数回)。
 bool neg;
 std::vector<u32> mag;
 if (geq(X, Y)) mag= diff(X, Y), neg= false;
 else mag= diff(Y, X), neg= true;
 while (neg) {
  Qh= dec(Qh);
  const int c= cmp_abs(mag, B);
  if (c > 0) bigint_internal::sub_abs_inplace(mag, B);
  else if (c == 0) mag.clear(), neg= false;
  else mag= diff(B, mag), neg= false;
 }
 while (geq(mag, B)) Qh= inc(Qh), bigint_internal::sub_abs_inplace(mag, B);
 Q= Qh, R= mag;
 trim(Q), trim(R);
}

// a / b (どちらも正、a >= b)。b の末尾は 0 でない。
inline void bz_divmod(const std::vector<u32>& a, const std::vector<u32>& b, std::vector<u32>& q, std::vector<u32>& r) {
 if (cmp_abs(a, b) < 0) {
  q.clear(), r= a;
  return;
 }
 const u32 d= u32(BASE / (bigint_internal::u64(b.back()) + 1));
 const std::vector<u32> A= bigint_internal::mul_small(a, d), B= bigint_internal::mul_small(b, d);
 const int m= int(B.size()), na= int(A.size());
 const int c= (na + m - 1) / m;
 q.assign(c * m, 0);
 std::vector<u32> R;
 for (int i= c - 1; i >= 0; --i) {
  std::vector<u32> chunk(m, 0);
  for (int j= 0; j < m; ++j) {
   const int idx= i * m + j;
   if (idx < na) chunk[j]= A[idx];
  }
  const std::vector<u32> X= add(shift(R, m), chunk);
  std::vector<u32> qi, ri;
  d2n1n(X, B, qi, ri);
  for (int j= 0; j < int(qi.size()) && j < m; ++j) q[i * m + j]= qi[j];
  R.swap(ri);
 }
 trim(q);
 u32 z;
 r= bigint_internal::divmod_small(R, d, z);
 assert(z == 0);
}
}  // namespace bigint_bz
