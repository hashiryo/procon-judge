#pragma once
#include "common.hpp"
#include "mylib/optimization/PiecewiseLinearConvex.hpp"

// 主形式。色 i から右へ運ぶ台数を変数にした凸関数を持ち、色 i で客が a_i 人来て
// 在庫が b_i k 台あるぶんを平行移動で、次の色まで運ぶ費用を絶対値で積む。左端
// から運び込めないことを add_inf で表し、最後に運び出す量 0 の値を読む。
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
      f.add_inf();
      for (int i = 0; i < n; ++i) {
        f.chmin_cum();
        f.shift(c.a[i] - c.b[i] * k);
        if (i + 1 < n) f.add_abs(c.xs[i + 1] - c.xs[i], 0);
      }
      ans[k - 1] = (i64)f(0).value();
      PLC::reset();
    }
  }

  const vector<i64> &answer() const { return ans; }
};
