#pragma once
// 実験版の exp (1/g を持ち回る形、_shared/ntt/fps_exp_reuse.hpp) を、実験中の単独の NTT の核 (3 本目、AVX2) で回す。
// NeoLibrary の fps::exp との比を見るための提出。
#include <vector>
#include "neo/base/core/algebra/ZMod.hpp"
#include "_shared/ntt/fps_exp_reuse.hpp"
#include "_shared/ntt/ntt_avx2.hpp"

struct Solver {
 using Z= ZMod<998244353>;
 std::vector<Z> a, b;

 explicit Solver(const std::vector<i64>& src): a(src.size()) {
  for (size_t i= 0; i < src.size(); ++i) a[i]= Z::raw((unsigned)src[i]);
 }

 void run() { b= fps_exp_reuse<NttAvx2>(a, int(a.size())); }

 std::vector<i64> answer() const {
  std::vector<i64> r(b.size());
  for (size_t i= 0; i < b.size(); ++i) r[i]= b[i].val();
  return r;
 }
};
