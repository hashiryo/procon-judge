#pragma once
#include "pj.hpp"
#include "neo/data_structure/segtree/AddSum.hpp"

// NeoLibrary の SegmentTree に monoid::AddSum<long long> を渡す。中身は差分を 2 本の 8 分木の接頭辞和に持つ形。
struct Solver {
  SegmentTree<monoid::AddSum<i64>> seg;
  explicit Solver(const vector<i64> &a) : seg(a) {}
  void add(int l, int r, i64 x) { seg.apply(l, r, x); }
  i64 sum(int l, int r) { return seg.prod(l, r); }
};
