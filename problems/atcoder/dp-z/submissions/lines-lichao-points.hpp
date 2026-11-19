#pragma once
#include "common.hpp"
#include "_shared/lines/lichao.hpp"

// 問いの x (h_2, ..., h_N) を先に渡した Li Chao 木 (LiChaoPoints)。問いは二分探索で葉を引いてから根へ上る。
struct Lines {
  LiChaoPoints t;
  explicit Lines(const vector<i64> &xs) : t(xs) {}
  void insert(i64 a, i64 b) { t.add_line(a, b); }
  i64 query(i64 x) { return t.query(x); }
};

using Solver = FrogSolver<Lines>;
