#pragma once
// 今の Library の SubProductTree (mylib/fft/SubProductTree.hpp) で多点評価する。
#include <vector>
#include "mylib/algebra/ModInt.hpp"
#include "mylib/fft/SubProductTree.hpp"

struct Solver {
 using Mint= ModInt<998244353>;
 std::vector<Mint> f, x;
 std::vector<i64> out;

 Solver(const std::vector<i64>& a, const std::vector<i64>& b): f(a.size()), x(b.size()) {
  for (size_t i= 0; i < a.size(); ++i) f[i]= a[i];
  for (size_t i= 0; i < b.size(); ++i) x[i]= b[i];
 }

 void run() {
  const std::vector<Mint> r= SubProductTree<Mint>(x).multi_eval(f);
  out.resize(r.size());
  for (size_t i= 0; i < r.size(); ++i) out[i]= r[i].val();
 }

 std::vector<i64> values() const { return out; }
};
