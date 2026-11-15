#pragma once
#include "pj.hpp"
#include "neo/data_structure/segtree/Assign.hpp"

// NeoLibrary の SegmentTree に monoid::AssignMin<int> を渡す (値は絶対値 10^9 以下で int に収まる)。中身は子を 16 個持つ
// 遅延セグ木で、値と代入を i32 で持つ。i64 で持つ lib-neo と比べる。
struct Solver {
  SegmentTree<monoid::AssignMin<int>> seg;
  explicit Solver(const vector<i64> &a) : seg(int(a.size()), [&](int i) { return int(a[i]); }) {}
  void assign(int l, int r, i64 x) { seg.apply(l, r, int(x)); }
  i64 minimum(int l, int r) { return seg.prod(l, r); }
};
