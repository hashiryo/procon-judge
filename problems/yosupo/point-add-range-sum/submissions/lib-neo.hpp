#pragma once
#include "pj.hpp"
#include "neo/data_structure/segtree/Sum.hpp"

// NeoLibrary の SegmentTree に monoid::Sum<long long> を渡す。中身は 8 分木の接頭辞和 (wide8 と同じ)。
struct Solver {
  SegmentTree<monoid::Sum<i64>> seg;
  explicit Solver(const vector<i64> &a) : seg(a) {}
  void add(int p, i64 x) { seg.mul(p, x); }
  i64 sum(int l, int r) { return seg.prod(l, r); }
};
