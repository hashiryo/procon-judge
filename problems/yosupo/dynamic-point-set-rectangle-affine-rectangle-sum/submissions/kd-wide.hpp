#pragma once
// KD 木 (kd_complete_core.hpp) を、外接矩形の広い方の軸で分ける形で使う。
// 深さで交互に分ける形 (kd-alt.hpp) と比べて、節の外接矩形が正方形に近づく。
#include "kd_complete_core.hpp"

struct Solver {
  KDComplete<true> kd;
  const vector<array<i64, 7>> &qs;
  vector<i64> ans;

  Solver(const vector<array<i64, 3>> &points,
         const vector<array<i64, 2>> &spots,
         const vector<array<i64, 7>> &queries)
      : kd(points, spots), qs(queries) {}

  void run() {
    ans.clear();
    kd.run(qs, ans);
  }

  const vector<i64> &answers() const { return ans; }
};
