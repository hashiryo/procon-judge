#pragma once
#include "common.hpp"
#include "_shared/lines/lichao_fn.hpp"

// lines-lichao-points-idx と同じ手順を、載せる関数の型をテンプレートで受ける核 (LiChaoPointsFn) に直線 (LineFn) を
// 渡して動かす。
struct Lines {
  LiChaoPointsFn<LineFn> t;
  int k = 0;
  explicit Lines(const vector<i64> &xs) : t(xs, LiChaoPointsFn<LineFn>::SortedUnique{}) {}
  void insert(i64 a, i64 b) { t.add(LineFn{a, b}); }
  i64 query(i64) { return t.query_index(k++); }
};

using Solver = FrogSolver<Lines>;
