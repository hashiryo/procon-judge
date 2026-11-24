#pragma once
#include "pj.hpp"
#include "neo/algo/optimize/line/LiChaoTree.hpp"

// NeoLibrary の LiChaoTreePoints (問いの x を先に渡す Li Chao 木)。問いは二分探索で点を引く。
struct Solver {
  LiChaoTreePoints<i64> t;

  Solver(const vector<array<i64, 2>> &lines, const vector<i64> &xs) : t(xs) {
    for (auto &e : lines) t.add_line(e[0], e[1]);
  }

  void insert(i64 a, i64 b) { t.add_line(a, b); }

  i64 query(i64 p) { return t.query(p); }
};
