#pragma once
#include "common.hpp"
#include "_shared/lines/cht_deque.hpp"

// lines-cht-deque と同じ包絡で、問いを二分探索で引く形。
struct Lines {
  MonotoneCHT c;
  void insert(i64 a, i64 b) { c.add_line(a, b); }
  i64 query(i64 x) { return c.query(x); }
};

using Solver = FrogSolver<Lines>;
