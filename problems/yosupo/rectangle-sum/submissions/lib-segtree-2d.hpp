#pragma once
// 今の Library の SegmentTree_2D (mylib/data_structure/SegmentTree_2D.hpp)。x で切ったセグメント木の各ノードに、その
// 範囲の点を y の順に並べたセグメント木を持つ (1 クエリ O(log^2 N))。yosupo-point-add-rectangle-sum の lib-segtree-2d
// と同じ形で、足す操作が無いだけ。
#include "pj.hpp"
#include "mylib/data_structure/SegmentTree_2D.hpp"
struct RangeSum {
 using T= i64;
 static T ti() { return 0; }
 static T op(T l, T r) { return l + r; }
};
struct Solver {
 SegmentTree_2D<int, RangeSum> seg;
 explicit Solver(const vector<array<int, 3>>& p): seg(p) {}
 i64 rect_sum(int l, int d, int r, int u) const { return seg.prod(l, r, d, u); }
};
