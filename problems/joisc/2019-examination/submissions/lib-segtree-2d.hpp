#pragma once
#include "common.hpp"
#include "mylib/data_structure/SegmentTree_2D.hpp"

// 2 次元セグメント木に Z の降順で生徒を足しながら、右上の領域の総和を問い合わせる。
// 1 点更新も領域の総和も O(log^2 N)。
struct Engine {
  SegmentTree_2D<int, RangeCount> seg;

  explicit Engine(const set<array<int, 2>> &points) : seg(points) {}

  void add(int x, int y) { seg.mul(x, y, 1); }

  int count(int x, int y) { return seg.prod(x, INF, y, INF); }
};

using Solver = OfflineSolver<Engine>;
