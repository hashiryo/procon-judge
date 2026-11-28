// 期待出力を作る参照実装。今の Library の FactorialPrecalculation (iv、fct、fiv の 3 本) を固定したもの。
// ジェネレータか base.cpp を直したときだけ作り直されるよう、ライブラリ以外は include しない。
#pragma once
#include <cstddef>
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
