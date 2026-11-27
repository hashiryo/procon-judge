#pragma once
// 今の Library の sample_points_shift (mylib/fft/sample_points_shift.hpp) で f(c), …, f(c + m - 1) を作る。
#include <vector>
#include "mylib/algebra/ModInt.hpp"
#include "mylib/fft/sample_points_shift.hpp"

struct Solver {
 using Mint= ModInt<998244353>;
 std::vector<Mint> y;
 Mint c;
 int m;
 std::vector<i64> out;

 Solver(const std::vector<i64>& a, i64 cv, int mv): y(a.size()), c(cv), m(mv) {
  for (size_t i= 0; i < a.size(); ++i) y[i]= a[i];
 }

 void run() {
  const std::vector<Mint> r= sample_points_shift<Mint>(y, c, m);
  out.resize(r.size());
  for (size_t i= 0; i < r.size(); ++i) out[i]= r[i].val();
 }

 std::vector<i64> values() const { return out; }
};
