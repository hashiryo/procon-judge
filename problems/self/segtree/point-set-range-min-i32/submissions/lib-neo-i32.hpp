#pragma once
#include "pj.hpp"
#include "neo/base/ds/range/segtree/Min.hpp"

// NeoLibrary の SegmentTree に monoid::Min<int> を渡す。中身は子を 16 個持つ最小の木で、値を i32 で持ち、16 個の塊が
// キャッシュの 1 行に収まる。i64 で持つ lib-neo と比べる。
struct Solver {
  SegmentTree<monoid::Min<int>> seg;
  explicit Solver(const vector<i64> &a) : seg(int(a.size()), [&](int i) { return int(a[i]); }) {}
  void set(int p, i64 x) { seg.set(p, int(x)); }
  i64 minimum(int l, int r) { return seg.prod(l, r); }
};
