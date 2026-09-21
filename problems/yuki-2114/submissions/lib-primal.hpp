#pragma once
#include "common.hpp"
#include "mylib/optimization/PiecewiseLinearConvex.hpp"

// 主形式。商 i から右へ持ち越す頂点数を変数にした凸関数を持ち、商 i に両側の
// 頂点が来るぶんを平行移動で、次の商まで持ち越す費用を絶対値で積む。左端から
// 持ち込めないことを add_inf で表し、最後に持ち越し 0 の値を読む。余りごとに
// 独立なので、ノードのプールは組ごとに使い直す。
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
      f.add_inf();
      for (int i = 0; i < n; ++i) {
        f.chmin_cum();
        f.shift(g.a[i] - g.b[i]);
        if (i + 1 < n) f.add_abs(g.xs[i + 1] - g.xs[i], 0);
      }
      ans += (i64)f(0).value();
      PLC::reset();
    }
  }

  i64 answer() const { return ans; }
};
