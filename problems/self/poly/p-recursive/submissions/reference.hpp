#pragma once
// 参照実装 (ライブラリを読まない)。多項式係数の線形漸化式 Σ_{i=0}^{d} p_i(n) a_{n+i} = 0 の第 N 項。
// Chudnovsky の baby-step/giant-step。遷移行列は、割り算を避けるため y_n = (Π_{i<n} p_d(i)) (a_n, ..., a_{n+d-1}) に対して
//   y_{n+1} = B(n) y_n,  B(n) の行 r < d-1 が p_d(n) e_{r+1}、最後の行が -p_i(n)
// と取る。ブロック多項式 P(x) = Π_{i<B} B(x+i) を P_{2h}(x) = P_h(x+h) P_h(x) で倍々に作り、
// x = 0, B, 2B, ... の値を部分積木と余りで評価して掛ける。最後に y の最後の成分を Π p_d(n) で割る。
// このファイルは base.cpp から include され、run(p, a0, N) を出す。
#include <algorithm>
#include <vector>
#include "_shared/ntt/_reference.hpp"

namespace ref_prec {
using ntt_reference::u32;
using ntt_reference::u64;
constexpr u32 M= 998244353;
using Poly= std::vector<u32>;

inline Poly mul(const Poly& a, const Poly& b) {
 if (a.empty() || b.empty()) return {};
 Poly c(a.size() + b.size() - 1);
 ntt_reference::convolve_range<M>(a.data(), int(a.size()), b.data(), int(b.size()), 0, int(c.size()), c.data());
 return c;
}
inline u32 pw(u32 a, u64 e) {
 u32 r= 1;
 for (; e; e>>= 1, a= u32(u64(a) * a % M))
  if (e & 1) r= u32(u64(r) * a % M);
 return r;
}
// f(x + c)。f(x+c) = Σ_j x^j/j! · Σ_{i>=j} a_i i! c^{i-j}/(i-j)!。
inline Poly shift(const Poly& f, u32 c) {
 const int n= int(f.size());
 if (n == 0 || c == 0) return f;
 std::vector<u32> fact(n), ifact(n);
 fact[0]= 1;
 for (int i= 1; i < n; ++i) fact[i]= u32(u64(fact[i - 1]) * i % M);
 ifact[n - 1]= pw(fact[n - 1], M - 2);
 for (int i= n - 1; i > 0; --i) ifact[i - 1]= u32(u64(ifact[i]) * i % M);
 Poly a(n), b(n);
 for (int i= 0; i < n; ++i) a[i]= u32(u64(f[n - 1 - i]) * fact[n - 1 - i] % M);
 b[0]= 1;
 u32 cp= 1;
 for (int k= 1; k < n; ++k) cp= u32(u64(cp) * c % M), b[k]= u32(u64(cp) * ifact[k] % M);
 const Poly t= mul(a, b);
 Poly g(n);
 for (int j= 0; j < n; ++j) g[j]= u32(u64(t[n - 1 - j]) * ifact[j] % M);
 return g;
}
// 1/f mod x^n (f[0] != 0)。
inline Poly inv_series(const Poly& f, int n) {
 Poly g(1, pw(f[0], M - 2));
 for (int m= 1; m < n;) {
  const int m2= std::min(2 * m, n);
  Poly t= mul(f.size() > size_t(m2) ? Poly(f.begin(), f.begin() + m2) : f, g);
  t.resize(m2);
  for (auto& x: t) x= x ? M - x : 0;  // 1 - f g
  t[0]= (t[0] + 2) % M;
  g= mul(g, t);
  g.resize(m2);
  m= m2;
 }
 return g;
}
// a mod b (b の先頭は 0 でない)。a が b より短ければそのまま。
inline Poly mod_poly(const Poly& a, const Poly& b) {
 if (a.size() < b.size()) return a;
 if (b.size() == 1) return {};
 const int k= int(a.size() - b.size()) + 1;
 Poly ar(a.rbegin(), a.rbegin() + k), br(b.rbegin(), b.rbegin() + b.size());
 const Poly qi= inv_series(br, k);
 Poly q= mul(ar, qi);
 q.resize(k);
 std::reverse(q.begin(), q.end());
 Poly qb= mul(q, b);
 Poly r(b.size() - 1);
 for (size_t i= 0; i < r.size(); ++i) r[i]= (i < a.size() ? a[i] : 0);
 for (size_t i= 0; i < qb.size() && i < r.size(); ++i) r[i]= (r[i] + M - qb[i]) % M;
 while (!r.empty() && r.back() == 0) r.pop_back();
 return r;
}
// 点 xs での値 (相異なる)。
inline std::vector<u32> multipoint(const Poly& f, const std::vector<u32>& xs) {
 const int m= int(xs.size());
 std::vector<u32> res(m);
 if (m == 0) return res;
 int sz= 1;
 while (sz < m) sz<<= 1;
 std::vector<Poly> tr(2 * sz);
 for (int i= 0; i < m; ++i) tr[sz + i]= {xs[i] ? M - xs[i] : 0, 1};
 for (int i= sz - 1; i >= 1; --i) {
  if (tr[2 * i].empty() || tr[2 * i + 1].empty()) {  // 点の無い側
   tr[i]= tr[2 * i].empty() ? tr[2 * i + 1] : tr[2 * i];
   continue;
  }
  tr[i]= mul(tr[2 * i], tr[2 * i + 1]);
 }
 struct Rec {
  const std::vector<Poly>& tr;
  int sz;
  std::vector<u32>& res;
  void go(int node, const Poly& f) const {
   if (node >= sz) {
    res[node - sz]= f.empty() ? 0 : f[0];
    return;
   }
   if (tr[2 * node].empty()) return;
   go(2 * node, mod_poly(f, tr[2 * node]));
   if (!tr[2 * node + 1].empty()) go(2 * node + 1, mod_poly(f, tr[2 * node + 1]));
  }
 };
 Rec{tr, sz, res}.go(1, mod_poly(f, tr[1]));
 return res;
}
struct Mat {
 int d;
 std::vector<Poly> e;
 Mat(int d_= 0): d(d_), e(size_t(d_) * d_) {}
 Poly& operator()(int i, int j) { return e[size_t(i) * d + j]; }
 const Poly& operator()(int i, int j) const { return e[size_t(i) * d + j]; }
};
inline Mat mat_mul(const Mat& a, const Mat& b) {
 Mat c(a.d);
 for (int i= 0; i < a.d; ++i)
  for (int j= 0; j < a.d; ++j) {
   Poly s;
   for (int k= 0; k < a.d; ++k) {
    if (a(i, k).empty() || b(k, j).empty()) continue;
    const Poly t= mul(a(i, k), b(k, j));
    if (s.size() < t.size()) s.resize(t.size());
    for (size_t x= 0; x < t.size(); ++x) s[x]= u32((s[x] + t[x]) % M);
   }
   c(i, j)= std::move(s);
  }
 return c;
}
inline Mat mat_shift(const Mat& a, u32 c) {
 Mat b(a.d);
 for (int i= 0; i < a.d * a.d; ++i) b.e[i]= shift(a.e[i], c);
 return b;
}
inline u32 eval(const Poly& f, u32 x) {
 u32 r= 0;
 for (int i= int(f.size()) - 1; i >= 0; --i) r= u32((u64(r) * x + (i < int(f.size()) ? f[i] : 0)) % M);
 return r;
}
}  // namespace ref_prec

// p[i][j] は p_i の n^j の係数、a0 は a_0..a_{d-1}。p_d(n) != 0 (0 <= n <= N-d) を仮定する。
inline u32 run(const std::vector<std::vector<u32>>& p, const std::vector<u32>& a0, u64 N) {
 using namespace ref_prec;
 const int d= int(p.size()) - 1;
 if (N < u64(d)) return a0[N];
 const u64 len= N - u64(d) + 1;
 Mat base(d);
 for (int r= 0; r + 1 < d; ++r) base(r, r + 1)= p[d];
 for (int i= 0; i < d; ++i) {
  base(d - 1, i)= p[i];
  for (auto& c: base(d - 1, i)) c= c ? M - c : 0;
 }
 Mat den(1);
 den(0, 0)= p[d];
 u64 B= 1;
 while (4 * B * B <= len) B*= 2;
 Mat cur= base;
 for (u64 h= 1; h < B; h*= 2) {
  cur= mat_mul(mat_shift(cur, u32(h % M)), cur);
  den= mat_mul(mat_shift(den, u32(h % M)), den);
 }
 const u64 q= len / B, r= len % B;
 std::vector<u32> pts(q);
 for (u64 j= 0; j < q; ++j) pts[j]= u32(u64(j % M) * (B % M) % M);
 std::vector<std::vector<u32>> vals(d * d);
 for (int i= 0; i < d * d; ++i) vals[i]= multipoint(cur.e[i], pts);
 const std::vector<u32> dvals= multipoint(den.e[0], pts);
 std::vector<u32> v(a0.begin(), a0.begin() + d), w(d);
 u32 D= 1;
 for (u64 j= 0; j < q; ++j) {
  D= u32(u64(D) * dvals[j] % M);
  for (int i= 0; i < d; ++i) {
   u32 s= 0;
   for (int k= 0; k < d; ++k) s= u32((s + u64(vals[i * d + k][j]) * v[k]) % M);
   w[i]= s;
  }
  v.swap(w);
 }
 for (u64 i= 0; i < r; ++i) {
  const u64 n= q * B + i;
  const u32 nk= u32(n % M);
  u32 s= 0;
  for (int k= 0; k < d; ++k) s= u32((s + u64(eval(base(d - 1, k), nk)) * v[k]) % M);
  for (int k= 0; k + 1 < d; ++k) w[k]= u32(u64(eval(base(k, k + 1), nk)) * v[k + 1] % M);
  w[d - 1]= s;
  v.swap(w);
  D= u32(u64(D) * eval(p[d], nk) % M);
 }
 return u32(u64(v[d - 1]) * pw(D, M - 2) % M);
}
