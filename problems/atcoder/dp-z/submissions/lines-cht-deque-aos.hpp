#pragma once
#include "common.hpp"
#include "_shared/lines/cht_deque_aos.hpp"

// 傾きと切片を 1 つの構造体に持つ両端キュー。別の配列に持つ形との速さの差を測るための提出。
struct Lines {
  MonotoneCHTAos c;
  void insert(i64 a, i64 b) { c.add_line(a, b); }
  i64 query(i64 x) { return c.query_inc(x); }
};

using Solver = FrogSolver<Lines>;
