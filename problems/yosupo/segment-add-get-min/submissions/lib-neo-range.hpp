#pragma once
#include "pj.hpp"
#include "neo/algo/optimize/line/LiChaoTree.hpp"

// NeoLibrary の LiChaoTree (x の範囲を渡す Li Chao 木) に線分を足す。問いの x を前もって知らなくてよい代わりに、
// LiChaoTreePoints より遅い。
struct Solver {
  LiChaoTree<i64> t;

  explicit Solver(const vector<array<i64, 4>> &segs) : t(-1000000000, 1000000001, 4 * ((int)segs.size() + 200000)) {
    for (auto &e : segs) t.add_segment(e[0], e[1], e[2], e[3]);
  }

  void insert(i64 l, i64 r, i64 a, i64 b) { t.add_segment(l, r, a, b); }

  i64 query(i64 p) { return t.query(p); }
};
