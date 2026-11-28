#pragma once
// 既定の核 (ntt.hpp が選ぶ AVX2。x86 は intrinsics、arm は SIMDe) で回す。
// 実験中の単独の NTT の核 (3 本目、AVX2) で回す。
#include <vector>
#include "neo/base/core/algebra/ZMod.hpp"
#include "neo/base/core/poly/poly_divmod_fast.hpp"
#include "neo/base/core/convolve/ntt.hpp"

struct Solver {
 using Z= ZMod<998244353>;
 std::vector<Z> f, g, q, r;

 Solver(const std::vector<i64>& a, const std::vector<i64>& b): f(a.size()), g(b.size()) {
  for (size_t i= 0; i < a.size(); ++i) f[i]= Z::raw((unsigned)a[i]);
  for (size_t i= 0; i < b.size(); ++i) g[i]= Z::raw((unsigned)b[i]);
 }

 void run() {
  auto qr= poly_divmod_fast(f, g);
  q.swap(qr.first), r.swap(qr.second);
 }

 std::vector<i64> quotient() const { return conv(q); }
 std::vector<i64> remainder() const { return conv(r); }

 private:
 static std::vector<i64> conv(const std::vector<Z>& v) {
  std::vector<i64> w(v.size());
  for (size_t i= 0; i < v.size(); ++i) w[i]= v[i].val();
  return w;
 }
};
