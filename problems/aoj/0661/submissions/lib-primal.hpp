#pragma once
#include "common.hpp"
#include "mylib/optimization/PiecewiseLinearConvex.hpp"

// 主形式。列をまたいで運ぶ個数を変数にした凸関数を持ち、運ぶ費用を絶対値で
// 積む。上向きと下向きの累積最小を別々に取るので、操作の数が多くなる。
struct Solver {
  int n;
  vector<array<i64, 2>> pts;
  i64 ans = 0;

  Solver(int n, const vector<array<i64, 2>> &pts) : n(n), pts(pts) {}

  void run() {
    Reduced r = reduce(n, pts);
    PiecewiseLinearConvex<int> f;
    int sum = 0;
    for (int i = 0; i < n; ++i) {
      f.add_abs(1, 0);
      f.add_abs(1, -sum);
      f.add_linear(-1);
      f.chmin_cum();
      f.add_linear(2);
      f.chmin_cum(true);
      f.add_linear(-1);
      f.shift(-r.cell[i][1]);
      sum += r.cell[i][0] + r.cell[i][1];
    }
    ans = r.base + (i64)f(0).value();
  }

  i64 answer() const { return ans; }
};
