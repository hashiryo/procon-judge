#pragma once
// NeoLibrary の multivariate_convolve (neo/base/core/convolve/multivariate.hpp、ランクの変数 chi) を、
// 実験の核 NttAvx2Splat (5 本目、AVX2 の 8 レーン、全レーン同じひねり) で回す。
// 多変数は核の dft と idft を直接使う。3 本目の dft は 2026-11-28 にひねりの表の正規化を直した。
#include <vector>
#include "neo/base/core/algebra/ZMod.hpp"
#include "neo/base/core/convolve/ntt.hpp"
#include "neo/base/core/convolve/multivariate.hpp"
#include "_shared/ntt/ntt_avx2_splat.hpp"

struct Solver {
 using Z= ZMod<998244353>;
 std::vector<int> dim;
 std::vector<Z> f, g, h;

 Solver(const std::vector<int>& d, const std::vector<i64>& a, const std::vector<i64>& b): dim(d), f(a.size()), g(b.size()) {
  for (size_t i= 0; i < a.size(); ++i) f[i]= Z::raw((unsigned)a[i]);
  for (size_t i= 0; i < b.size(); ++i) g[i]= Z::raw((unsigned)b[i]);
 }

 void run() { h= multivariate_convolve<NttAvx2Splat>(dim, f, g); }

 std::vector<i64> answer() const {
  std::vector<i64> r(h.size());
  for (size_t i= 0; i < h.size(); ++i) r[i]= h[i].val();
  return r;
 }
};
