#pragma once
// NeoLibrary の永続 Li Chao 木 (neo/algo/optimize/line/PersistentLiChaoTree.hpp) で line_add_get_min を解く。
// 版は残さず、直前の版から add_line しては捨てる。永続の写しの分だけ非永続の LiChaoTree より遅い。
#include <cstddef>
#include "pj.hpp"
#include "neo/algo/optimize/line/PersistentLiChaoTree.hpp"

struct Solver {
  using P = PersistentLiChaoTree<i64>;
  P cur;
  explicit Solver(const vector<array<i64, 2>> &lines) : cur(-1000000000, 1000000001) {
    // 全体に足す直線 1 本は高々 31 節。N + Q は 2 * 10^5 以下。
    P::reserve((lines.size() + 200000) * 32 + 1);
    for (auto &e : lines) cur = cur.add_line(e[0], e[1]);
  }
  void insert(i64 a, i64 b) { cur = cur.add_line(a, b); }
  i64 query(i64 p) { return cur.query(p); }
};
