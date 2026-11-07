#pragma once
#include "pj.hpp"
#include "neo/data_structure/segtree/Assign.hpp"

// NeoLibrary の SegmentTree に monoid::AssignSum<i64> を渡す。中身は子を 16 個持つ遅延セグ木。
struct Solver {
  SegmentTree<monoid::AssignSum<i64>> seg;
  explicit Solver(const vector<i64> &a) : seg(a) {}
  void assign(int l, int r, i64 x) { seg.apply(l, r, x); }
  i64 sum(int l, int r) { return seg.prod(l, r); }
};
