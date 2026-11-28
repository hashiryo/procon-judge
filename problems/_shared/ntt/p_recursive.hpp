#pragma once
// 多項式係数の線形漸化式 (P-recursive) Σ_{i=0}^{d} p_i(n) a_{n+i} = 0 (n >= 0) の第 N 項。
// p[i][j] は p_i の n^j の係数で、a0 は a_0..a_{d-1}。p_d(n) != 0 (0 <= n <= N - d) を仮定する。
//
// 算法は Chudnovsky の baby-step/giant-step (手間は N の平方根の準線形)。
// v_n = (a_n, ..., a_{n+d-1}) とすると p_d(n) v_{n+1} = M(n) v_n (M の最後の行が -p_i(n)、行 r < d-1 が e_{r+1})。
// 割り算を避けるため y_n = (Π_{i<n} p_d(i)) v_n と置くと y_{n+1} = B(n) y_n で、
// B(n) の行 r < d-1 が p_d(n) e_{r+1}、最後の行が -p_i(n)。よって
//   a_N = (e_{d-1}^T B(N-d) ... B(0) v_0) / (Π_{n=0}^{N-d} p_d(n))。
// 積は、ブロック多項式 P(x) = Π_{i<B} B(x+i) を P_{2h}(x) = P_h(x+h) P_h(x) で倍々に作り (テイラーシフト)、
// x = 0, B, 2B, ... での値を多点評価で出して掛ける。B はおよそ (N - d + 1) の平方根。
// 核はテンプレート引数 Conv で受け、畳み込みの共通の口 (convolve.hpp) の convolve / 多点評価に渡す。
// K は体で、法 (標数) は N - d + 1 より大きいこと (テイラーシフトが 1/j! を使う)。
#include <algorithm>
#include <cassert>
#include <vector>
#include "neo/base/core/convolve/convolve.hpp"
#include "neo/base/core/poly/multipoint.hpp"
#include "neo/base/core/poly/sample_shift.hpp"
#include "neo/base/core/poly/taylor_shift.hpp"

namespace p_recursive_internal {
template <class K> using Poly= std::vector<K>;
// d x d の多項式行列 (行が先)。
template <class K> struct PMat {
 int d;
 std::vector<Poly<K>> e;
 PMat(int d_= 0): d(d_), e(size_t(d_) * d_) {}
 Poly<K>& operator()(int i, int j) { return e[size_t(i) * d + j]; }
 const Poly<K>& operator()(int i, int j) const { return e[size_t(i) * d + j]; }
};
template <class Conv, class K> PMat<K> mul(const PMat<K>& a, const PMat<K>& b) {
 const int d= a.d;
 PMat<K> c(d);
 for (int i= 0; i < d; ++i)
  for (int j= 0; j < d; ++j) {
   Poly<K> s;
   for (int k= 0; k < d; ++k) {
    if (a(i, k).empty() || b(k, j).empty()) continue;
    const Poly<K> t= convolve<Conv>(a(i, k), b(k, j));
    if (s.size() < t.size()) s.resize(t.size());
    for (int x= 0; x < int(t.size()); ++x) s[x]+= t[x];
   }
   c(i, j)= std::move(s);
  }
 return c;
}
template <class Conv, class K> PMat<K> shift(const PMat<K>& a, K c) {
 PMat<K> b(a.d);
 for (int i= 0; i < a.d * a.d; ++i) b.e[i]= taylor_shift<Conv>(a.e[i], c);
 return b;
}
template <class K> K eval(const Poly<K>& f, K x) {
 K r= K();
 for (int i= int(f.size()) - 1; i >= 0; --i) r= r * x + f[i];
 return r;
}
}  // namespace p_recursive_internal

namespace p_recursive {
template <class Conv= convolve_internal::Auto, class K>
K nth(const std::vector<std::vector<K>>& p, const std::vector<K>& a0, unsigned long long N) {
 using namespace p_recursive_internal;
 const int d= int(p.size()) - 1;
 assert(d >= 1 && int(a0.size()) >= d);
 if (N < (unsigned long long)d) return a0[N];
 const unsigned long long len= N - (unsigned long long)d + 1;  // 掛ける遷移行列の数
 PMat<K> base(d);
 for (int r= 0; r + 1 < d; ++r) base(r, r + 1)= p[d];
 for (int i= 0; i < d; ++i) {
  base(d - 1, i)= p[i];
  for (auto& c: base(d - 1, i)) c= -c;
 }
 PMat<K> den(1);
 den(0, 0)= p[d];
 unsigned long long B= 1;
 while (2 * B * B <= len) B*= 2;
 PMat<K> cur= base;
 for (unsigned long long h= 1; h < B; h*= 2) {
  cur= mul<Conv>(shift<Conv>(cur, K(h)), cur);
  den= mul<Conv>(shift<Conv>(den, K(h)), den);
 }
 const unsigned long long q= len / B, r= len % B;
 std::vector<K> pts(q);
 for (unsigned long long j= 0; j < q; ++j) pts[j]= K(j * B);
 std::vector<std::vector<K>> vals(d * d);
 for (int i= 0; i < d * d; ++i) vals[i]= multipoint_eval<Conv>(cur.e[i], pts);
 const std::vector<K> dvals= multipoint_eval<Conv>(den(0, 0), pts);
 std::vector<K> v(a0.begin(), a0.begin() + d), w(d);
 K D(1);
 for (unsigned long long j= 0; j < q; ++j) {
  D*= dvals[j];
  for (int i= 0; i < d; ++i) {
   K s= K();
   for (int k= 0; k < d; ++k) s+= vals[i * d + k][j] * v[k];
   w[i]= s;
  }
  v.swap(w);
 }
 for (unsigned long long i= 0; i < r; ++i) {
  const unsigned long long n= q * B + i;
  const K nk= K(n);
  K s= K();
  for (int k= 0; k < d; ++k) s+= eval(base(d - 1, k), nk) * v[k];
  for (int k= 0; k + 1 < d; ++k) w[k]= eval(base(k, k + 1), nk) * v[k + 1];
  w[d - 1]= s;
  v.swap(w);
  D*= eval(p[d], nk);
 }
 return v[d - 1] / D;
}
// 値ベースの Bostan–Gaudry–Schost 版。ブロック多項式を係数で作らず、等差数列 X = jk 上の値のまま倍々にする。
//   M_{2t}(jk) = M_t(jk + t) M_t(jk)
// で、M_t(jk + t) は「X で t ずらす」= 添字で t/k ずらす標本点のずらし 1 回で作る。定数係数 (すべての次数が 0) は
// 行列の冪で進める。口と仮定は nth と同じ (e >= 1 の道では k を使うので、法は 2 e k + 1 より大きいこと)。
template <class Conv= convolve_internal::Auto, class K>
K nth_values(const std::vector<std::vector<K>>& p, const std::vector<K>& a0, unsigned long long N) {
 using namespace p_recursive_internal;
 const int d= int(p.size()) - 1;
 assert(d >= 1 && int(a0.size()) >= d);
 if (N < (unsigned long long)d) return a0[N];
 const unsigned long long len= N - (unsigned long long)d + 1;
 PMat<K> base(d);
 for (int r= 0; r + 1 < d; ++r) base(r, r + 1)= p[d];
 for (int i= 0; i < d; ++i) {
  base(d - 1, i)= p[i];
  for (auto& c: base(d - 1, i)) c= -c;
 }
 int edeg= int(p[d].size()) - 1;
 for (int i= 0; i < d; ++i) edeg= std::max(edeg, int(p[i].size()) - 1);
 if (edeg == 0) {  // 定数係数: 遷移行列が一定なので、行列の冪で進める
  std::vector<K> mx(d * d), my(d * d), t(d * d);
  for (int i= 0; i < d; ++i)
   for (int j= 0; j < d; ++j) mx[i * d + j]= eval(base(i, j), K());
  for (int i= 0; i < d * d; ++i) my[i]= K(), t[i]= K();
  for (int i= 0; i < d; ++i) my[i * d + i]= K(1);
  for (unsigned long long e= len; e; e>>= 1) {
   if (e & 1) {
    for (int i= 0; i < d; ++i)
     for (int j= 0; j < d; ++j) {
      K s= K();
      for (int kk= 0; kk < d; ++kk) s+= my[i * d + kk] * mx[kk * d + j];
      t[i * d + j]= s;
     }
    my.swap(t);
   }
   for (int i= 0; i < d; ++i)
    for (int j= 0; j < d; ++j) {
     K s= K();
     for (int kk= 0; kk < d; ++kk) s+= mx[i * d + kk] * mx[kk * d + j];
     t[i * d + j]= s;
    }
   mx.swap(t);
  }
  K D(1), pwbase= p[d][0];
  for (unsigned long long x= len; x; x>>= 1, pwbase*= pwbase)
   if (x & 1) D*= pwbase;
  K r= K();
  for (int i= 0; i < d; ++i) r+= my[(d - 1) * d + i] * a0[i];
  return r / D;
 }
 // ブロック長 k = 2^s。q = floor(len / k) が e k + 1 以下になる最小の 2 冪。
 unsigned long long k= 1;
 while (k < len && (unsigned long long)edeg * k * k < len) k<<= 1;
 const unsigned long long q= len / k, r= len % k;
 const int nent= d * d + 1;  // 最後の 1 本が分母 p_d
 std::vector<std::vector<K>> val(nent);
 for (int e= 0; e < d * d; ++e) {
  val[e].resize(edeg + 1);
  for (int j= 0; j <= edeg; ++j) val[e][j]= eval(base.e[e], K((unsigned long long)j * k));
 }
 val[d * d].resize(edeg + 1);
 for (int j= 0; j <= edeg; ++j) val[d * d][j]= eval(p[d], K((unsigned long long)j * k));
 const K kinv= K(k).inv();
 std::vector<std::vector<K>> bs(nent), tl(nent), t2(nent), nv(nent);
 std::vector<K> A(d * d), Bv(d * d), C(d * d);
 for (unsigned long long t= 1; t < k; t<<= 1) {
  const int cnt= int(edeg * t) + 1;
  for (int e= 0; e < nent; ++e) {
   bs[e]= sample_points_shift<Conv>(val[e], K(t) * kinv, cnt);  // F(t/k + j)
   tl[e]= sample_points_shift<Conv>(val[e], K(cnt), cnt);       // F(cnt + j)
   t2[e]= sample_points_shift<Conv>(bs[e], K(cnt), cnt);        // F(t/k + cnt + j)
  }
  for (int e= 0; e < nent; ++e) nv[e].resize(2 * cnt - 1);
  for (int j= 0; j < 2 * cnt - 1; ++j) {
   for (int i= 0; i < d; ++i)
    for (int kk= 0; kk < d; ++kk) {
     A[i * d + kk]= j < cnt ? bs[i * d + kk][j] : t2[i * d + kk][j - cnt];
     Bv[i * d + kk]= j < cnt ? val[i * d + kk][j] : tl[i * d + kk][j - cnt];
    }
   for (int i= 0; i < d; ++i)
    for (int kk= 0; kk < d; ++kk) {
     K s= K();
     for (int l= 0; l < d; ++l) s+= A[i * d + l] * Bv[l * d + kk];
     C[i * d + kk]= s;
    }
   for (int i= 0; i < d * d; ++i) nv[i][j]= C[i];
   const K fd= j < cnt ? val[d * d][j] : tl[d * d][j - cnt];
   const K ft= j < cnt ? bs[d * d][j] : t2[d * d][j - cnt];
   nv[d * d][j]= fd * ft;
  }
  val.swap(nv);
 }
 std::vector<K> v(a0.begin(), a0.begin() + d), w(d);
 K D(1);
 for (unsigned long long j= 0; j < q; ++j) {
  D*= val[d * d][j];
  for (int i= 0; i < d; ++i) {
   K s= K();
   for (int kk= 0; kk < d; ++kk) s+= val[i * d + kk][j] * v[kk];
   w[i]= s;
  }
  v.swap(w);
 }
 for (unsigned long long i= 0; i < r; ++i) {
  const unsigned long long n= q * k + i;
  const K nk= K(n);
  K s= K();
  for (int kk= 0; kk < d; ++kk) s+= eval(base(d - 1, kk), nk) * v[kk];
  for (int kk= 0; kk + 1 < d; ++kk) w[kk]= eval(base(kk, kk + 1), nk) * v[kk + 1];
  w[d - 1]= s;
  v.swap(w);
  D*= eval(p[d], nk);
 }
 return v[d - 1] / D;
}
}  // namespace p_recursive
