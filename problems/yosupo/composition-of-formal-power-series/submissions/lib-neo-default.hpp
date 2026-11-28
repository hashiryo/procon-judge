#pragma once
// 既定の核 (ntt.hpp が選ぶ AVX2。x86 は intrinsics、arm は SIMDe) で回す。
#include <vector>
#include "neo/base/core/algebra/ZMod.hpp"
#include "neo/base/core/poly/compose.hpp"
#include "neo/base/core/convolve/ntt.hpp"

struct Solver {
 using Z= ZMod<998244353>;
 int n;
 std::vector<Z> f, g;
 std::vector<i64> out;

 Solver(const std::vector<i64>& a, const std::vector<i64>& b): n(int(a.size())), f(a.size()), g(b.size()) {
  for (size_t i= 0; i < a.size(); ++i) f[i]= Z::raw((unsigned)a[i]);
  for (size_t i= 0; i < b.size(); ++i) g[i]= Z::raw((unsigned)b[i]);
 }

 void run() {
  const std::vector<Z> r= compose(f, g, n);
  out.resize(n);
  for (int i= 0; i < n; ++i) out[i]= r[i].val();
 }

 std::vector<i64> coeffs() const { return out; }
};
