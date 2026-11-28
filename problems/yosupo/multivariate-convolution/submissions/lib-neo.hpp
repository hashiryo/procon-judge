#pragma once
// NeoLibrary の multivariate_convolve (neo/base/core/convolve/multivariate.hpp、ランクの変数 chi) を、
// 核 Ntt で回す。
#include <vector>
#include "neo/base/core/algebra/ZMod.hpp"
#include "neo/base/core/convolve/ntt.hpp"
#include "neo/base/core/convolve/multivariate.hpp"

struct Solver {
 using Z= ZMod<998244353>;
 std::vector<int> dim;
 std::vector<Z> f, g, h;

 Solver(const std::vector<int>& d, const std::vector<i64>& a, const std::vector<i64>& b): dim(d), f(a.size()), g(b.size()) {
  for (size_t i= 0; i < a.size(); ++i) f[i]= Z::raw((unsigned)a[i]);
  for (size_t i= 0; i < b.size(); ++i) g[i]= Z::raw((unsigned)b[i]);
 }

 void run() { h= multivariate_convolve<Ntt>(dim, f, g); }

 std::vector<i64> answer() const {
  std::vector<i64> r(h.size());
  for (size_t i= 0; i < h.size(); ++i) r[i]= h[i].val();
  return r;
 }
};
