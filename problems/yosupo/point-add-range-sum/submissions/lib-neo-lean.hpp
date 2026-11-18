#pragma once
#include "_sum_exp.hpp"

// lib-neo の切り分け。NeoLibrary の SegmentTree<monoid::Sum<i64>> の写しで、作る段だけを、一時の配列を作らず表を埋めずに
// 確保する形 (_sum_exp.hpp の SumExp<false>) に替える。
struct Solver {
  SumExp<false> seg;
  explicit Solver(const vector<i64> &a) : seg((int)a.size(), [&](int i) { return a[i]; }) {}
  void add(int p, i64 x) { seg.mul(p, x); }
  i64 sum(int l, int r) { return seg.prod(l, r); }
};
