#pragma once
// NeoLibrary の fps::inv (Newton 法、neo/base/core/poly/fps.hpp) を、実験中の単独の NTT の核 (8 本目、葉を 4 枚まとめて取る形だけを足したもの) で回す。
#include <vector>
#include "neo/base/core/algebra/ZMod.hpp"
#include "neo/base/core/poly/fps.hpp"
#include "_shared/ntt/ntt_avx2_leaf4.hpp"

struct Solver {
 using Z= ZMod<998244353>;
 std::vector<Z> a, b;

 explicit Solver(const std::vector<i64>& src): a(src.size()) {
  for (size_t i= 0; i < src.size(); ++i) a[i]= Z::raw((unsigned)src[i]);
 }

 void run() { b= fps::inv<NttAvx2Leaf4>(a, int(a.size())); }

 std::vector<i64> answer() const {
  std::vector<i64> r(b.size());
  for (size_t i= 0; i < b.size(); ++i) r[i]= b[i].val();
  return r;
 }
};
