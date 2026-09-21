#pragma once
#include "pj.hpp"
#include "mylib/optimization/PiecewiseLinearConvex.hpp"

// 共役形式。傾きを変数にした関数を持つので、買う費用と捨てる費用が定義域の制限
// (add_inf) に、運ぶ費用が窓の最小 (chmin_slide_win) に入れ替わる。操作の数が
// 主形式より少ない。
struct Solver {
  int x, y, z;
  vector<array<int, 2>> ab;
  i64 ans = 0;

  Solver(int x, int y, int z, const vector<array<int, 2>> &ab) : x(x), y(y), z(z), ab(ab) {}

  void run() {
    PiecewiseLinearConvex<int> f;
    for (auto &e : ab) {
      f.add_linear(e[0] - e[1]);
      f.add_inf(false, -y);
      f.add_inf(true, x);
      f.chmin_slide_win(-z, z);
    }
    ans = -(i64)f.min().value();
  }

  i64 answer() const { return ans; }
};
