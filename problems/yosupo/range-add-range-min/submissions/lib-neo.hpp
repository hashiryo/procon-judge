#pragma once
#include "pj.hpp"
#include "neo/data_structure/segtree/AddMin.hpp"

// NeoLibrary の SegmentTree に monoid::AddMin<long long> を渡す。中身は作用を配らない 16 分木 (wide3 と同じ)。
struct Solver {
  SegmentTree<monoid::AddMin<i64>> seg;
  explicit Solver(const vector<i64> &a) : seg(a) {}
  void add(int l, int r, i64 x) { seg.apply(l, r, x); }
  i64 minimum(int l, int r) { return seg.prod(l, r); }
};
