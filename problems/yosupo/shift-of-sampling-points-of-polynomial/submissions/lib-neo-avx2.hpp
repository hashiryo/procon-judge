#pragma once
// NeoLibrary の sample_points_shift (neo/base/core/poly/sample_shift.hpp) を、実験中の単独の NTT の核 (3 本目、AVX2) で回す。
#include <vector>
#include "neo/base/core/algebra/ZMod.hpp"
#include "neo/base/core/poly/sample_shift.hpp"
#include "_shared/ntt/ntt_avx2.hpp"

struct Solver {
 using Z= ZMod<998244353>;
 std::vector<Z> y;
 Z c;
 int m;
 std::vector<i64> out;

 Solver(const std::vector<i64>& a, i64 cv, int mv): y(a.size()), c(Z::raw((unsigned)cv)), m(mv) {
  for (size_t i= 0; i < a.size(); ++i) y[i]= Z::raw((unsigned)a[i]);
 }

 void run() {
  const std::vector<Z> r= sample_points_shift<NttAvx2>(y, c, m);
  out.resize(r.size());
  for (size_t i= 0; i < r.size(); ++i) out[i]= r[i].val();
 }

 std::vector<i64> values() const { return out; }
};
