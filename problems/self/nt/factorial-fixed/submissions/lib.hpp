// 今の Library の FactorialPrecalculation (iv、fct、fiv の 3 本) で解く。参照実装と同じ算法。
#pragma once
#include <vector>
#include "mylib/algebra/ModInt.hpp"
#include "mylib/counting/FactorialPrecalculation.hpp"

struct Solver {
 std::vector<unsigned> run(unsigned N, const std::vector<unsigned>& ns, const std::vector<unsigned>& rs) {
  using mint= ModInt<998244353>;
  (void)N;
  FactorialPrecalculation<mint>::reset();
  std::vector<unsigned> out;
  out.reserve(ns.size());
  for (std::size_t i= 0; i < ns.size(); ++i) out.push_back((unsigned)FactorialPrecalculation<mint>::nCr((int)ns[i], (int)rs[i]).val());
  return out;
 }
};
