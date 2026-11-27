#pragma once
// 今の Library の SubProductTree (mylib/fft/SubProductTree.hpp) で補間する。
#include <vector>
#include "mylib/algebra/ModInt.hpp"
#include "mylib/fft/SubProductTree.hpp"

struct Solver {
 using Mint= ModInt<998244353>;
 std::vector<Mint> x, y;
 std::vector<i64> out;

 Solver(const std::vector<i64>& a, const std::vector<i64>& b): x(a.size()), y(b.size()) {
  for (size_t i= 0; i < a.size(); ++i) x[i]= a[i];
  for (size_t i= 0; i < b.size(); ++i) y[i]= b[i];
 }

 void run() {
  const std::vector<Mint> f= SubProductTree<Mint>(x).interpolate(y);
  out.resize(f.size());
  for (size_t i= 0; i < f.size(); ++i) out[i]= f[i].val();
 }

 std::vector<i64> coeffs() const { return out; }
};
