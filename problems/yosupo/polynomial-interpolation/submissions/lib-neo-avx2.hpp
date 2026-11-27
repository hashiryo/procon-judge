#pragma once
// NeoLibrary の補間 (neo/base/core/poly/multipoint.hpp) を、実験中の単独の NTT の核 (3 本目、AVX2) で回す。
#include <vector>
#include "neo/base/core/algebra/ZMod.hpp"
#include "neo/base/core/poly/multipoint.hpp"
#include "_shared/ntt/ntt_avx2.hpp"

struct Solver {
 using Z= ZMod<998244353>;
 std::vector<Z> x, y;
 std::vector<i64> out;

 Solver(const std::vector<i64>& a, const std::vector<i64>& b): x(a.size()), y(b.size()) {
  for (size_t i= 0; i < a.size(); ++i) x[i]= Z::raw((unsigned)a[i]);
  for (size_t i= 0; i < b.size(); ++i) y[i]= Z::raw((unsigned)b[i]);
 }

 void run() {
  const std::vector<Z> f= interpolate<NttAvx2>(x, y);
  out.resize(f.size());
  for (size_t i= 0; i < f.size(); ++i) out[i]= f[i].val();
 }

 std::vector<i64> coeffs() const { return out; }
};
