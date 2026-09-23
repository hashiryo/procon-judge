#pragma once
#include "common.hpp"
#include "mylib/data_structure/SegmentTree_2D.hpp"

// 2 次元セグメント木。x のセグメント木の各節点に、そこに入る点の y だけを持つ
// セグメント木を載せる。長方形の最小は O(log^2 N)。
struct Index {
  SegmentTree_2D<int, RangeMin> seg;

  explicit Index(const vector<array<int, 3>> &xyw) : seg(xyw) {}

  int min_in(int x0, int x1, int y0, int y1) const { return seg.prod(x0, x1, y0, y1); }
};

using Solver = DetourSolver<Index>;
