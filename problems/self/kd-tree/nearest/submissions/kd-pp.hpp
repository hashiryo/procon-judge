#pragma once
#include "pj.hpp"
#include "neo/base/ds/kd/KdBall.hpp"

// neo の KdBall (1 点 1 ノードの KD 木) で最近傍を引く。
struct Solver {
  const vector<array<i64, 2>> &pts_;
  const vector<array<i64, 2>> &qs_;
  vector<i64> ans_;

  Solver(const vector<array<i64, 2>> &pts, const vector<array<i64, 2>> &qs) : pts_(pts), qs_(qs) {}

  void run() {
    const int n = (int)pts_.size();
    vector<array<int, 2>> pt(n);
    vector<int> id(n);
    for (int i = 0; i < n; ++i) pt[i] = {(int)pts_[i][0], (int)pts_[i][1]}, id[i] = i;
    const KdBall<int, 2> kd(pt, id);
    ans_.clear();
    ans_.reserve(qs_.size());
    for (const auto &q : qs_) ans_.push_back(kd.nearest2({(int)q[0], (int)q[1]}));
  }

  const vector<i64> &answers() const { return ans_; }
};
