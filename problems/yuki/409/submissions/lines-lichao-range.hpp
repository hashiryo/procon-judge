#pragma once
#include "common.hpp"
#include "_shared/lines/lichao.hpp"

// 費用を展開した直線を、範囲 [1, N] を渡す Li Chao 木 (LiChaoRange) に入れる。
struct Lines {
  LiChaoRange t;
  explicit Lines(const vector<i64> &xs) : t(1, (i64)xs.size() + 1, (int)xs.size() + 1) {}
  void insert(i64 a, i64 b) { t.add_line(a, b); }
  i64 query(i64 x) { return t.query(x); }
};

using Solver = LineSolver<Lines>;
