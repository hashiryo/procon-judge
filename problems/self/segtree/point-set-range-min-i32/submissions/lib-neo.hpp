#pragma once
#include "pj.hpp"
#include "neo/data_structure/segtree/Min.hpp"

// NeoLibrary の SegmentTree に monoid::Min<long long> を渡す。中身は子を 16 個持つ最小の木。
struct Solver {
  SegmentTree<monoid::Min<i64>> seg;
  explicit Solver(const vector<i64> &a) : seg(a) {}
  void set(int p, i64 x) { seg.set(p, x); }
  i64 minimum(int l, int r) { return seg.prod(l, r); }
};
