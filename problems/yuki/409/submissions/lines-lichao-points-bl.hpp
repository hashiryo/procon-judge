#pragma once
#include "common.hpp"
#include "_shared/lines/lichao.hpp"

// lines-lichao-points と同じ木で、追加を分岐を使わずに葉まで下る形 (add_line_full) にする。
struct Lines {
  LiChaoPoints t;
  explicit Lines(const vector<i64> &xs) : t(xs, LiChaoPoints::SortedUnique{}) {}
  void insert(i64 a, i64 b) { t.add_line_full(a, b); }
  i64 query(i64 x) { return t.query_index((int)x - 1); }
};

using Solver = LineSolver<Lines>;
