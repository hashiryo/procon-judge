#pragma once
#include "pj.hpp"
#include "neo/algo/optimize/line/LiChaoTree.hpp"

// NeoLibrary の LiChaoTreePoints (問いの x を先に渡す Li Chao 木) に線分を足す。
struct Solver {
  LiChaoTreePoints<i64> t;

  Solver(const vector<array<i64, 4>> &segs, const vector<i64> &xs) : t(xs) {
    for (auto &e : segs) t.add_segment(e[0], e[1], e[2], e[3]);
  }

  void insert(i64 l, i64 r, i64 a, i64 b) { t.add_segment(l, r, a, b); }

  i64 query(i64 p) { return t.query(p); }
};
