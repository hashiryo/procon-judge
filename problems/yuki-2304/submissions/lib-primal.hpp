#pragma once
#include "pj.hpp"
#include "mylib/optimization/PiecewiseLinearConvex.hpp"

// 主形式。「i 番目まで見て最後の値が x のときの費用」を x の凸関数として持ち、
// 累積最小と 1 だけの平行移動で「1 つ前より真に大きい」制約を、絶対値で操作回数
// を順に積む。
struct Solver {
  vector<i64> a;
  i64 ans = 0;

  explicit Solver(const vector<i64> &a) : a(a) {}

  void run() {
    PiecewiseLinearConvex<i64> f;
    for (i64 v : a) {
      f.chmin_cum();
      f.shift(1);
      f.add_abs(1, v);
    }
    ans = (i64)f.min().value();
  }

  i64 answer() const { return ans; }
};
