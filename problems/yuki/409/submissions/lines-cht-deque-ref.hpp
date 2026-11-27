#pragma once
#include "common.hpp"
#include "_shared/lines/cht_deque_ref.hpp"

// 費用を展開した直線を、頭を戻さない両端キュー (MonotoneCHTRef) に入れる。問いの x は 1 ずつ増える。
struct Lines {
  MonotoneCHTRef c;
  void insert(i64 a, i64 b) { c.add_line(a, b); }
  i64 query(i64 x) { return c.query_inc(x); }
};

using Solver = LineSolver<Lines>;
