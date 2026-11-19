#pragma once
#include "common.hpp"
#include "_shared/lines/lichao.hpp"

// lines-lichao-points-idx と同じく k 回目の問いを葉 k で引き、追加を分岐を使わずに葉まで下る形 (add_line_full) にする。
struct Lines {
  LiChaoPoints t;
  int k = 0;
  explicit Lines(const vector<i64> &xs) : t(xs, LiChaoPoints::SortedUnique{}) {}
  void insert(i64 a, i64 b) { t.add_line_full(a, b); }
  i64 query(i64) { return t.query_index(k++); }
};

using Solver = FrogSolver<Lines>;
