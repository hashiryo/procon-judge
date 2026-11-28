#pragma once
#include "pj.hpp"
#include "mylib/data_structure/KDTree.hpp"

// 今の Library の KDTree で最近傍を引く。構築も run() の中で行う (どちらの提出も同じ条件にする)。
struct Solver {
  const vector<array<i64, 2>> &pts_;
  const vector<array<i64, 2>> &qs_;
  vector<i64> ans_;

  Solver(const vector<array<i64, 2>> &pts, const vector<array<i64, 2>> &qs) : pts_(pts), qs_(qs) {}

  void run() {
    KDTree<i64, 2> kdt(pts_);
    ans_.clear();
    ans_.reserve(qs_.size());
    for (const auto &q : qs_) {
      const auto p = kdt.nearest_neighbor(q[0], q[1]);
      const i64 dx = p[0] - q[0], dy = p[1] - q[1];
      ans_.push_back(dx * dx + dy * dy);
    }
  }

  const vector<i64> &answers() const { return ans_; }
};
