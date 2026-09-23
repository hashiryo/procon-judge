#pragma once
#include "common.hpp"
#include "mylib/number_theory/DirichletSeries.hpp"

// ディリクレ級数の畳み込みで phi の累積和を作る。O(N^(2/3) log^(1/3) N)。
struct Solver {
  i64 n;
  Mint ans;

  explicit Solver(i64 n) : n(n) {}

  void run() { ans = get_phi<Mint>(n).sum(); }

  i64 answer() const { return ans.val(); }
};
