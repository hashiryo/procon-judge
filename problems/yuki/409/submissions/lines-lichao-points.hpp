#pragma once
#include "common.hpp"
#include "_shared/lines/lichao.hpp"

// 費用を展開した直線を、問いの x (1, 2, ..., N) を先に渡した Li Chao 木 (LiChaoPoints) に入れる。x は葉の添字
// そのものなので、二分探索をせずに x - 1 の葉から根へ上る。
struct Lines {
  LiChaoPoints t;
  explicit Lines(const vector<i64> &xs) : t(xs, LiChaoPoints::SortedUnique{}) {}
  void insert(i64 a, i64 b) { t.add_line(a, b); }
  i64 query(i64 x) { return t.query_index((int)x - 1); }
};

using Solver = LineSolver<Lines>;
