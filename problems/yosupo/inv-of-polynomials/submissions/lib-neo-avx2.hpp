#pragma once
// NeoLibrary の poly_extgcd (half-gcd) を、実験中の単独の NTT の核 (3 本目、AVX2) で回す。
#include <vector>
#include "neo/base/core/algebra/ZMod.hpp"
#include "neo/base/core/poly/poly_extgcd.hpp"
#include "_shared/ntt/ntt_avx2.hpp"

struct Solver {
 using Z= ZMod<998244353>;
 std::vector<Z> f, g, x, y, d;

 Solver(const std::vector<i64>& a, const std::vector<i64>& b): f(a.size()), g(b.size()) {
  for (size_t i= 0; i < a.size(); ++i) f[i]= Z::raw((unsigned)a[i]);
  for (size_t i= 0; i < b.size(); ++i) g[i]= Z::raw((unsigned)b[i]);
 }

 void run() { d= poly_extgcd<NttAvx2>(f, g, x, y); }

 bool impossible() const { return d.size() != 1; }

 std::vector<i64> answer() const {
  if (d.size() != 1 || x.empty()) return {};
  const Z id0= d[0].inv();
  std::vector<i64> r(x.size());
  for (size_t i= 0; i < x.size(); ++i) r[i]= (x[i] * id0).val();
  return r;
 }
};
