#pragma once
#include "common.hpp"
#include "_shared/lines/cht_deque.hpp"

// 傾きが単調に減る順に足す両端キューの CHT (MonotoneCHT)。問いの x は増えるので、先頭から進める query_inc で引く。
struct Lines {
  MonotoneCHT c;
  void insert(i64 a, i64 b) { c.add_line(a, b); }
  i64 query(i64 x) { return c.query_inc(x); }
};

using Solver = FrogSolver<Lines>;
