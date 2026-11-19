#pragma once
#include "common.hpp"
#include "_shared/lines/lichao.hpp"

// lichao-points と同じ木で、追加を分岐を使わずに葉まで下る形 (add_line_full) にし、問いの葉を分岐のない二分探索で
// 引く (query_bl)。
struct Lines {
  LiChaoPoints t;

  explicit Lines(const vector<i64> &xs) : t(xs) {}

  void insert(i64 a, i64 b) { t.add_line_full(a, b); }

  i64 query(i64 x) const { return t.query_bl(x); }
};

using Solver = StatueSolver<Lines>;
