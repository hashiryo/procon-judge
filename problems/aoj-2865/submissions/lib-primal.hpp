#pragma once
#include "pj.hpp"
#include "mylib/optimization/PiecewiseLinearConvex.hpp"

// 主形式。位置を変数にした凸関数を持ち、移動の費用を絶対値として積む。
// 定義域の両端を無限で押さえてから、幅 1 の窓で最小を取って進める。
struct Solver {
  vector<i64> d, g;
  i64 ans = 0;

  Solver(const vector<i64> &d, const vector<i64> &g) : d(d), g(g) {}

  void run() {
    PiecewiseLinearConvex<i64> f;
    f.add_inf(), f.add_inf(true);
    for (int i = 0; i < (int)g.size(); ++i) {
      if (i) f.add_abs(d[i - 1], 0);
      f.add_linear(-g[i]);
      f.chmin_slide_win(-1, 1);
      f.add_linear(g[i]);
      f.add_const(g[i]);
    }
    ans = (i64)f(0).value();
  }

  i64 answer() const { return ans; }
};
