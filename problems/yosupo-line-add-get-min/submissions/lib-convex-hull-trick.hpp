#pragma once
#include "pj.hpp"
#include "mylib/optimization/ConvexHullTrick.hpp"

// 直線の下側凸包を平衡二分探索木で持つ。追加のたびに隠れた直線を消すので、
// 直線の本数ぶんのノードしか残らない。
struct Solver {
  ConvexHullTrick<i64, MINIMIZE> cht;

  explicit Solver(const vector<array<i64, 2>> &lines) {
    for (auto &e : lines) cht.insert(e[0], e[1]);
  }

  void insert(i64 a, i64 b) { cht.insert(a, b); }

  i64 query(i64 p) { return cht.query(p); }
};
