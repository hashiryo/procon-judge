#pragma once
#include "common.hpp"
#include "mylib/optimization/ConvexHullTrick.hpp"

// 直線の下側凸包を平衡二分探索木で持つ。追加のたびに隠れた直線を消すので、
// 残るノードは凸包を作る直線の本数ぶんだけ。傾きの順は問わない。
struct Lines {
  ConvexHullTrick<i64, MINIMIZE> cht;

  void insert(i64 a, i64 b) { cht.insert(a, b); }

  i64 query(i64 x) const { return cht.query(x); }
};

using Solver = StatueSolver<Lines>;
