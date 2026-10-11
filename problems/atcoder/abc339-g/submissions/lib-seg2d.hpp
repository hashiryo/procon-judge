#pragma once
// 今の Library の SegmentTree_2D (mylib/data_structure/SegmentTree_2D.hpp) に、点 (i, A_i) を重み A_i で載せ、
// [l, r) x [0, x] の和を取る (1 クエリ O(log^2 N))。raw のときの lib-seg2d.cpp と同じ中身。
#include <tuple>
#include "pj.hpp"
#include "mylib/data_structure/SegmentTree_2D.hpp"
struct RSQ {
 using T= long long;
 static T ti() { return 0; }
 static T op(T a, T b) { return a + b; }
};
struct Solver {
 SegmentTree_2D<int, RSQ> seg;
 static vector<tuple<int, int, long long>> points(const vector<int>& a) {
  vector<tuple<int, int, long long>> ps(a.size());
  for(size_t i= 0; i < a.size(); ++i) ps[i]= {int(i), a[i], a[i]};
  return ps;
 }
 explicit Solver(const vector<int>& a): seg(points(a)) {}
 i64 sum_le(int l, int r, int x) const { return seg.prod(l, r, 0, x + 1); }
};
