#pragma once
#include "common.hpp"
#include "mylib/data_structure/SegmentTree_2D.hpp"

// マージソート木。x で切ったセグメント木の各ノードに、その範囲の点を y 順に
// 並べた BIT を持つ。1 クエリ O(log^2 N)。
struct Solver {
  SegmentTree_2D<int, RangeSum> seg;

  Solver(const vector<array<i64, 3>> &points, const vector<array<int, 2>> &spots)
      : seg(to_map(points, spots)) {}

  void add(int x, int y, i64 w) { seg.mul(x, y, w); }

  i64 rect_sum(int l, int d, int r, int u) { return seg.prod(l, r, d, u); }
};
