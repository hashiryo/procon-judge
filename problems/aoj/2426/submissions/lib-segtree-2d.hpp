#pragma once
// 今の Library の SegmentTree_2D (mylib/data_structure/SegmentTree_2D.hpp) に、重み 1 の点を載せて数える。x で切った
// セグメント木の各ノードに、その範囲の点を y の順に並べたセグメント木を持つ (1 クエリ O(log^2 n))。
#include "pj.hpp"
#include "mylib/data_structure/SegmentTree_2D.hpp"
struct RangeSum {
 using T= int;
 static T ti() { return 0; }
 static T op(T l, T r) { return l + r; }
};
struct Solver {
 SegmentTree_2D<int, RangeSum> seg;
 static vector<array<int, 3>> weighted(const vector<array<int, 2>>& p) {
  vector<array<int, 3>> w(p.size());
  for(size_t i= 0; i < p.size(); ++i) w[i]= {p[i][0], p[i][1], 1};
  return w;
 }
 explicit Solver(const vector<array<int, 2>>& p): seg(weighted(p)) {}
 int count(int l, int d, int r, int u) const { return seg.prod(l, r, d, u); }
};
