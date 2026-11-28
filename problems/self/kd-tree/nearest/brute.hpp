#pragma once
// この問題の愚直解。各クエリで全部の点との距離の 2 乗を調べて最小を出す。KD 木を使わないので、
// 参照実装とは別の考え方になる。O(NQ) なので小さい入力でだけ使う
// (pj testdata crosscheck --brute brute.hpp)。
#include "pj.hpp"

struct Solver {
  const vector<array<i64, 2>> &pts_;
  const vector<array<i64, 2>> &qs_;
  vector<i64> ans_;

  Solver(const vector<array<i64, 2>> &pts, const vector<array<i64, 2>> &qs) : pts_(pts), qs_(qs) {}

  void run() {
    ans_.clear();
    for (const auto &q : qs_) {
      i64 best = -1;
      for (const auto &p : pts_) {
        const i64 dx = p[0] - q[0], dy = p[1] - q[1];
        const i64 d2 = dx * dx + dy * dy;
        if (best < 0 || d2 < best) best = d2;
      }
      ans_.push_back(best);
    }
  }

  const vector<i64> &answers() const { return ans_; }
};
