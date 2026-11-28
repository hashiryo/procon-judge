#pragma once
// KD 木 (kd_complete_core.hpp) を、深さで x と y を交互に分ける形で使う。
// 判定サイトの模範解答と同じ分け方。点を葉に置き、節の添字はセグメント木と同じにする。
#include "kd_complete_core.hpp"

struct Solver {
  KDComplete<false> kd;
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
