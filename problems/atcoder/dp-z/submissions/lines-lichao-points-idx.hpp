#pragma once
#include "common.hpp"
#include "_shared/lines/lichao.hpp"

// lines-lichao-points と同じ木で、問いの葉の添字を二分探索で引かない形。h は狭義に増えるので、問いの x の列は
// 並べて重複を除いたものそのもので、k 回目の問いは葉 k。
struct Lines {
  LiChaoPoints t;
  int k = 0;
  explicit Lines(const vector<i64> &xs) : t(xs, LiChaoPoints::SortedUnique{}) {}
  void insert(i64 a, i64 b) { t.add_line(a, b); }
  i64 query(i64) { return t.query_index(k++); }
};

using Solver = FrogSolver<Lines>;
