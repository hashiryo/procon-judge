#pragma once
#include "common.hpp"
#include "_shared/lines/cht_set.hpp"

// 傾きの順を問わない multiset の CHT (SetCHT、今の Library の ConvexHullTrick と同じ作り) の節を、pmr のプールから取る形。
// 消した節を使い回すので、追加のたびの malloc が無くなる。最大なので、傾きと切片と答えの符号を反転して最小で持つ。
struct Lines {
  SetCHT<true> c;
  void insert(i64 a, i64 b) { c.add_line(-a, -b); }
  i64 query(i64 x) const { return -c.query(x); }
};

using Solver = BallSolver<Lines>;
