#pragma once
#include "pj.hpp"
#include "mylib/optimization/PiecewiseLinearConvex.hpp"

// 共役形式。傾きを変数にした関数を持つので、主形式の累積最小が定義域の制限
// (add_inf) に、平行移動が 1 次の項に、絶対値が窓の最小 (chmin_slide_win) に
// 入れ替わる。
struct Solver {
  vector<i64> a;
  i64 ans = 0;

  explicit Solver(const vector<i64> &a) : a(a) {}

  void run() {
    PiecewiseLinearConvex<i64> f;
    f.add_inf();
    for (i64 v : a) {
      f.add_inf(true);
      f.add_linear(1);
      f.add_linear(-v);
      f.chmin_slide_win(-1, 1);
      f.add_linear(v);
    }
    ans = -(i64)f(0).value();
  }

  i64 answer() const { return ans; }
};
