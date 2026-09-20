#pragma once
#include "common.hpp"
#include "mylib/optimization/PiecewiseLinearConvex.hpp"

// 共役形式。傾きを変数にした関数を持つので、主形式で絶対値だったところが
// 窓の最小に入れ替わり、定義域の制限が add_inf で書ける。積む操作が少ない。
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
      f.chmin_slide_win(-1, 1);
      f.add_linear(sum);
      f.chmin_slide_win(-1, 1);
      f.add_linear(-sum);
      f.add_inf(true, 1);
      f.add_inf(false, -1);
      f.add_linear(-r.cell[i][1]);
      sum += r.cell[i][0] + r.cell[i][1];
    }
    ans = r.base - (i64)f.min().value();
  }

  i64 answer() const { return ans; }
};
