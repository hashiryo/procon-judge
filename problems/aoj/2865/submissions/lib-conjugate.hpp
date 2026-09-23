#pragma once
#include "pj.hpp"
#include "mylib/optimization/PiecewiseLinearConvex.hpp"

// 共役形式。傾きを変数にした関数を持つので、主形式で絶対値だったところが
// 窓の最小に、窓の最小だったところが絶対値に入れ替わる。積む操作が少ない。
struct Solver {
  vector<i64> d, g;
  i64 ans = 0;

  Solver(const vector<i64> &d, const vector<i64> &g) : d(d), g(g) {}

  void run() {
    PiecewiseLinearConvex<i64> f;
    for (int i = 0; i < (int)g.size(); ++i) {
      if (i) f.chmin_slide_win(-d[i - 1], d[i - 1]);
      f.add_abs(1, g[i]);
      f.add_const(-g[i]);
    }
    ans = (i64)-f.min().value();
  }

  i64 answer() const { return ans; }
};
