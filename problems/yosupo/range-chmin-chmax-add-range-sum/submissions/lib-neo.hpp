#pragma once
#include "pj.hpp"
#include "neo/data_structure/segtree/ChminChmaxAddSum.hpp"

// NeoLibrary の SegmentTree に monoid::ChminChmaxAddSum<long long> を渡す。中身はこの 3 つの操作に特化した Beats (beats と同じ形)。
struct Solver {
  using M = monoid::ChminChmaxAddSum<i64>;
  SegmentTree<M> seg;
  explicit Solver(const vector<i64> &a) : seg(int(a.size()), [&](int i) { return a[i]; }) {}
  void chmin(int l, int r, i64 b) { seg.apply(l, r, M::chmin(b)); }
  void chmax(int l, int r, i64 b) { seg.apply(l, r, M::chmax(b)); }
  void add(int l, int r, i64 b) { seg.apply(l, r, M::add(b)); }
  i64 sum(int l, int r) { return seg.prod(l, r).sum; }
};
