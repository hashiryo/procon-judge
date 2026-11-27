#pragma once
// NeoLibrary の fps::sqrt (位数を外に出して exp(log / 2)、neo/base/core/poly/fps.hpp) を、実験中の単独の NTT の核 (3 本目、AVX2) で回す。
#include <vector>
#include "neo/base/core/algebra/ZMod.hpp"
#include "neo/base/core/poly/fps.hpp"
#include "_shared/ntt/ntt_avx2.hpp"

struct Solver {
 using Z= ZMod<998244353>;
 std::vector<Z> a, b;

 explicit Solver(const std::vector<i64>& src): a(src.size()) {
  for (size_t i= 0; i < src.size(); ++i) a[i]= Z::raw((unsigned)src[i]);
 }

 void run() { b= fps::sqrt<NttAvx2>(a, int(a.size())); }

 std::vector<i64> answer() const {
  if (b.empty()) return {};  // 平方根が無い
  std::vector<i64> r(b.size());
  for (size_t i= 0; i < b.size(); ++i) r[i]= b[i].val();
  return r;
 }
};
