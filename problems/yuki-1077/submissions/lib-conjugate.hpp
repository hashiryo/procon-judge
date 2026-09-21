#pragma once
#include "pj.hpp"
#include "mylib/optimization/PiecewiseLinearConvex.hpp"

// 共役形式。傾きを変数にした関数を持つので、主形式の累積最小が定義域の制限
// (add_inf) に、絶対値が窓の最小 (chmin_slide_win) に入れ替わる。
struct Solver {
  vector<i64> y;
  i64 ans = 0;

  explicit Solver(const vector<i64> &y) : y(y) {}

  void run() {
    PiecewiseLinearConvex<int> f;
    f.add_inf(), f.add_inf(true);
    for (i64 v : y) {
      f.add_inf(true);
      f.add_linear(-(int)v);
      f.chmin_slide_win(-1, 1);
      f.add_linear((int)v);
    }
    ans = -(i64)f(0).value();
  }

  i64 answer() const { return ans; }
};
