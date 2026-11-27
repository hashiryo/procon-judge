#pragma once
#include "common.hpp"
#include "_shared/lines/cht_deque_ref.hpp"

// 参照実装と同じ、頭を戻さない両端キュー (MonotoneCHTRef)。問いの x は増えるので query_inc で引く。
struct Lines {
  MonotoneCHTRef c;
  void insert(i64 a, i64 b) { c.add_line(a, b); }
  i64 query(i64 x) { return c.query_inc(x); }
};

using Solver = FrogSolver<Lines>;
