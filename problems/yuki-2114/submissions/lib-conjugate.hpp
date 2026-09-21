#pragma once
#include "common.hpp"
#include "mylib/optimization/PiecewiseLinearConvex.hpp"

// 共役形式。傾きを変数にした関数を持つので、主形式の累積最小が定義域の制限
// (add_inf) に、平行移動が 1 次の項に、絶対値が窓の最小 (chmin_slide_win) に
// 入れ替わる。答えは最小値の符号を変えたもの。余りごとに独立なので、ノードの
// プールは組ごとに使い直す。
struct Solver {
  i64 k;
  vector<i64> b, r;
  i64 ans = 0;

  Solver(i64 k, const vector<i64> &b, const vector<i64> &r) : k(k), b(b), r(r) {}

  void run() {
    using PLC = PiecewiseLinearConvex<i64>;
    const bool blue_small = b.size() < r.size();
    vector<Group> groups;
    if (!group_by_residue(k, blue_small ? b : r, blue_small ? r : b, groups)) {
      ans = -1;
      return;
    }
    ans = 0;
    for (auto &g : groups) {
      const int n = (int)g.xs.size();
      PLC f;
      f.add_inf(true);
      for (int i = 0; i < n; ++i) {
        f.add_inf(true);
        f.add_linear(g.a[i] - g.b[i]);
        if (i + 1 < n) {
          const i64 d = g.xs[i + 1] - g.xs[i];
          f.chmin_slide_win(-d, d);
        }
      }
      ans -= (i64)f.min().value();
      PLC::reset();
    }
  }

  i64 answer() const { return ans; }
};
