#pragma once
#include "common.hpp"
#include "_shared/lines/lichao.hpp"

// 範囲を渡す Li Chao 木 (LiChaoRange)。範囲は問題文の制約の [1, 10^6]。
struct Lines {
  LiChaoRange t{1, 1000001, 200000};
  void insert(i64 a, i64 b) { t.add_line(a, b); }
  i64 query(i64 x) { return t.query(x); }
};

using Solver = FrogSolver<Lines>;
