#pragma once
#include "pj.hpp"
#include "_shared/lines/lichao.hpp"

// lichao-points と同じ木で、追加を分岐を使わずに葉まで下る形 (add_line_full) にし、問いの葉を分岐のない二分探索で
// 引く (query_bl)。
struct Solver {
  LiChaoPoints t;

  Solver(const vector<array<i64, 2>> &lines, const vector<i64> &xs) : t(xs) {
    for (auto &e : lines) t.add_line_full(e[0], e[1]);
  }

  void insert(i64 a, i64 b) { t.add_line_full(a, b); }

  i64 query(i64 p) { return t.query_bl(p); }
};
