#pragma once
// P-recursive の第 N 項、係数を作って多点評価する版 (実験。採ったのは NeoLibrary の
// neo/algo/compute/poly/p_recursive.hpp の値ベースの形で、こちらは書き比べの負けた側)。
// 遷移行列は、行 r < d-1 が p_d(n) e_{r+1}、最後の行が -p_i(n)。
// ブロック多項式 P(x) = Π_{i<B} B(x+i) を P_{2h}(x) = P_h(x+h) P_h(x) で倍々に作り (テイラーシフト)、
// x = 0, B, 2B, ... の値を多点評価で出して掛ける。最後に Π p_d(n) で 1 回だけ割る。
#include <algorithm>
#include <cassert>
#include <vector>
#include "neo/base/core/convolve/convolve.hpp"
#include "neo/base/core/poly/multipoint.hpp"
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
}  // namespace p_recursive
