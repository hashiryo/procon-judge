#pragma once
#include "common.hpp"
#include "mylib/optimization/PiecewiseLinearConvex.hpp"

// 共役形式。傾きを変数にした関数を持つので、主形式の累積最小が定義域の制限
// (add_inf) に、平行移動が 1 次の項に、絶対値が窓の最小 (chmin_slide_win) に
// 入れ替わる。答えは最小値の符号を変えたもの。
// k ごとに木を作り直すので、ノードのプールも k ごとに使い直す。
struct Solver {
  vector<i64> a, b, ans;

  Solver(const vector<i64> &a, const vector<i64> &b) : a(a), b(b) {}

  void run() {
    using PLC = PiecewiseLinearConvex<i64>;
    Counts c = count_colors(a, b);
    const int n = (int)c.xs.size(), m = (int)a.size();
    ans.assign(m, 0);
    for (int k = 1; k <= m; ++k) {
      PLC f;
      for (int i = 0; i < n; ++i) {
        f.add_inf(true);
        f.add_linear(c.a[i] - c.b[i] * k);
        if (i + 1 < n) {
          const i64 d = c.xs[i + 1] - c.xs[i];
          f.chmin_slide_win(-d, d);
        }
      }
      ans[k - 1] = -(i64)f.min().value();
      PLC::reset();
    }
  }

  const vector<i64> &answer() const { return ans; }
};
