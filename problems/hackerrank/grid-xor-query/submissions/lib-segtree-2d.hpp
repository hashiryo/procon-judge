#pragma once
#include "common.hpp"
#include "mylib/data_structure/SegmentTree_2D.hpp"

// 2 次元セグメント木。x のセグメント木の各節点に、そこに入る点の y だけを持つ
// セグメント木を載せる。長方形の XOR は O(log^2 N)。範囲は半開区間で渡す。
struct Index {
  SegmentTree_2D<int, RangeXor> seg;

  explicit Index(const vector<array<int, 3>> &pts) : seg(pts) {}

  int query(int a, int b, int c, int d) const { return seg.prod(a, b + 1, c, d + 1); }
};

using Solver = GridSolver<Index>;
