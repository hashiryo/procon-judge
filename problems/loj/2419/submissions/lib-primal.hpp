#pragma once
#include "pj.hpp"
#include "mylib/optimization/PiecewiseLinearConvex.hpp"

// 主形式。右へ持ち越す土の量を変数にした凸関数を持ち、花壇ごとに差分の平行移動、
// 買う費用と捨てる費用を 2 方向の累積最小で、運ぶ費用を絶対値で積む。
struct Solver {
  int x, y, z;
  vector<array<int, 2>> ab;
  i64 ans = 0;

  Solver(int x, int y, int z, const vector<array<int, 2>> &ab) : x(x), y(y), z(z), ab(ab) {}

  void run() {
    PiecewiseLinearConvex<int> f;
    f.add_inf(), f.add_inf(true);
    for (auto &e : ab) {
      f.shift(e[0] - e[1]);
      f.add_linear(y);
      f.chmin_cum(true);
      f.add_linear(-x - y);
      f.chmin_cum();
      f.add_linear(x);
      f.add_abs(z, 0);
    }
    ans = (i64)f(0).value();
  }

  i64 answer() const { return ans; }
};
