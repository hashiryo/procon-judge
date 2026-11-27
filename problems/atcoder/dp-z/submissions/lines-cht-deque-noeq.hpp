#pragma once
#include "common.hpp"
#include "_shared/lines/cht_deque_noeq.hpp"

// 頭を戻さず、同じ傾きの判定も持たない両端キュー。dp-z の傾き -2 h は狭義に減るので、この核が使える。
// 同じ傾きの判定の代金を測るための提出。
struct Lines {
  MonotoneCHTNoEq c;
  void insert(i64 a, i64 b) { c.add_line(a, b); }
  i64 query(i64 x) { return c.query_inc(x); }
};

using Solver = FrogSolver<Lines>;
