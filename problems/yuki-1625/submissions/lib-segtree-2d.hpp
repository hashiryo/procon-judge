#pragma once
#include "common.hpp"
#include "mylib/data_structure/SegmentTree_2D.hpp"

// 2 次元セグメント木。x のセグメント木の各節点に、そこに入る点の y だけを持つ
// セグメント木を載せる。長方形の最大も 1 点更新も O(log^2 N)。
struct Engine {
  SegmentTree_2D<int, RangeMax> seg;

  explicit Engine(const map<array<int, 2>, i64> &points) : seg(points) {}

  void mul(int l, int r, i64 s) { seg.mul(l, r, s); }

  i64 max_in(int l, int r) { return seg.prod(l, r + 1, l, r + 1); }
};

using Solver = TriangleSolver<Engine>;
