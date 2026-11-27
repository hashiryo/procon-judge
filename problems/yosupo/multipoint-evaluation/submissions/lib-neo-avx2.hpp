#pragma once
// NeoLibrary の多点評価 (neo/base/core/poly/multipoint.hpp) を、実験中の単独の NTT の核 (3 本目、AVX2) で回す。
#include <vector>
#include "neo/base/core/algebra/ZMod.hpp"
#include "neo/base/core/poly/multipoint.hpp"
#include "_shared/ntt/ntt_avx2.hpp"

struct Solver {
 using Z= ZMod<998244353>;
 std::vector<Z> f, x;
 std::vector<i64> out;

 Solver(const std::vector<i64>& a, const std::vector<i64>& b): f(a.size()), x(b.size()) {
  for (size_t i= 0; i < a.size(); ++i) f[i]= Z::raw((unsigned)a[i]);
  for (size_t i= 0; i < b.size(); ++i) x[i]= Z::raw((unsigned)b[i]);
 }

 void run() {
  const std::vector<Z> r= multipoint_eval<NttAvx2>(f, x);
  out.resize(r.size());
  for (size_t i= 0; i < r.size(); ++i) out[i]= r[i].val();
 }

 std::vector<i64> values() const { return out; }
};
