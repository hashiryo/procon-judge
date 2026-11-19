#pragma once
#include "common.hpp"
#include "mylib/optimization/ConvexHullTrick.hpp"

// 費用を展開した直線を、今の Library の ConvexHullTrick (平衡二分探索木の凸包、傾きの順を問わない) に入れる。
struct Lines {
  ConvexHullTrick<i64, MINIMIZE> cht;
  void insert(i64 a, i64 b) { cht.insert(a, b); }
  i64 query(i64 x) { return cht.query(x); }
};

using Solver = LineSolver<Lines>;
