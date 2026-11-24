#pragma once
#include "pj.hpp"
#include "neo/algo/optimize/line/LiChaoTree.hpp"

// NeoLibrary の LiChaoTree (x の範囲を渡す Li Chao 木)。
struct Solver {
  LiChaoTree<i64> t;

  explicit Solver(const vector<array<i64, 2>> &lines) : t(-1000000000, 1000000001, (int)lines.size() + 200000) {
    for (auto &e : lines) t.add_line(e[0], e[1]);
  }

  void insert(i64 a, i64 b) { t.add_line(a, b); }

  i64 query(i64 p) { return t.query(p); }
};
