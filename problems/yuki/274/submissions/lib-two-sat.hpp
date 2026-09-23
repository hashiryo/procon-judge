#pragma once
#include "common.hpp"
#include "mylib/misc/TwoSatisfiability.hpp"

// 有向 2-SAT。x_i != x_j を (x_i nand x_j) と (!x_i nand !x_j) に、x_i = x_j を
// (!x_i nand x_j) と (x_i nand !x_j) に直して含意グラフに入れ、強連結成分分解で
// 充足可能か見る。制約を全部入れてから解くので途中で止まれず、辺の数が全対に
// 比例する。
struct Solver {
  int m;
  vector<i64> l, r;
  bool ok = false;

  Solver(int m, const vector<i64> &l, const vector<i64> &r) : m(m), l(l), r(r) {}

  void run() {
    const int n = (int)l.size();
    TwoSatisfiability sat(n);
    for (int i = 0; i < n; ++i)
      for (int j = 0; j < i; ++j) {
        auto [same, differ] = constraint(m, l[i], r[i], l[j], r[j]);
        if (differ) sat.add_nand(i, j), sat.add_nand(sat.neg(i), sat.neg(j));
        if (same) sat.add_nand(sat.neg(i), j), sat.add_nand(i, sat.neg(j));
      }
    ok = !sat.solve().empty();
  }

  bool answer() const { return ok; }
};
