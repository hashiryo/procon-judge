#pragma once
#include "pj.hpp"
#include "_shared/kd-tree/kd_ball_2d.hpp"

// KD 木の共通の核 (点を葉に置く完全 2 分木) で最近傍を引く。近い側の子から降り、
// 外接矩形までの距離が今の最善以上になったら打ち切る。
struct Solver {
  const vector<array<i64, 2>> &pts_;
  const vector<array<i64, 2>> &qs_;
  vector<i64> ans_;

  Solver(const vector<array<i64, 2>> &pts, const vector<array<i64, 2>> &qs) : pts_(pts), qs_(qs) {}

  void run() {
    const int n = (int)pts_.size();
    vector<int> x(n), y(n), id(n);
    for (int i = 0; i < n; ++i) x[i] = (int)pts_[i][0], y[i] = (int)pts_[i][1], id[i] = i;
    const KdBall2D kd(x, y, id);
    ans_.clear();
    ans_.reserve(qs_.size());
    for (const auto &q : qs_) ans_.push_back(kd.nearest2((int)q[0], (int)q[1]));
  }

  const vector<i64> &answers() const { return ans_; }
};
