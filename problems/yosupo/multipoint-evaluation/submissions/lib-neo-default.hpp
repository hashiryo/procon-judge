#pragma once
// 既定の核 (ntt.hpp が選ぶ AVX2。x86 は intrinsics、arm は SIMDe) で回す。
#include <vector>
#include "neo/base/core/algebra/ZMod.hpp"
#include "neo/base/core/poly/multipoint.hpp"
#include "neo/base/core/convolve/ntt.hpp"

struct Solver {
 using Z= ZMod<998244353>;
 std::vector<Z> f, x;
 std::vector<i64> out;

 Solver(const std::vector<i64>& a, const std::vector<i64>& b): f(a.size()), x(b.size()) {
  for (size_t i= 0; i < a.size(); ++i) f[i]= Z::raw((unsigned)a[i]);
  for (size_t i= 0; i < b.size(); ++i) x[i]= Z::raw((unsigned)b[i]);
 }

 void run() {
  const std::vector<Z> r= multipoint_eval(f, x);
  out.resize(r.size());
  for (size_t i= 0; i < r.size(); ++i) out[i]= r[i].val();
 }

 std::vector<i64> values() const { return out; }
};
