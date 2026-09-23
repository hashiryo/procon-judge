#pragma once
#include "pj.hpp"
#include "mylib/optimization/PiecewiseLinearConvex.hpp"

// 主形式。「i 番目まで見て最後の高さが x のときの費用」を x の凸関数として持ち、
// 単調増加の制約を累積最小で、移動距離を絶対値で順に積む。
struct Solver {
  vector<i64> y;
  i64 ans = 0;

  explicit Solver(const vector<i64> &y) : y(y) {}

  void run() {
    PiecewiseLinearConvex<int> f;
    for (i64 v : y) {
      f.chmin_cum();
      f.add_abs(1, (int)v);
    }
    ans = (i64)f.min().value();
  }

  i64 answer() const { return ans; }
};
