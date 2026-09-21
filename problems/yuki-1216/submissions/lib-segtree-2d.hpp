#pragma once
#include "common.hpp"
#include "mylib/data_structure/SegmentTree_2D.hpp"

// 2 次元セグメント木。x のセグメント木の各節点に、そこに入る点の y だけを持つ
// セグメント木を載せる。長方形の総和も 1 点更新も O(log^2 N)。
struct Engine {
  SegmentTree_2D<i64, RangeCount> seg;

  explicit Engine(const set<array<i64, 2>> &points) : seg(points) {}

  void add(i64 x, i64 y, int delta) { seg.mul(x, y, delta); }

  int count(i64 x0, i64 x1, i64 ymax) { return seg.prod(x0, x1, 0, ymax + 1); }
};

using Solver = LanternSolver<Engine>;
