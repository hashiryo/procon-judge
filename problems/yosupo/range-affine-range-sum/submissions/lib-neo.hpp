#pragma once
#include "pj.hpp"
#include "neo/data_structure/segtree/Affine.hpp"

// NeoLibrary の SegmentTree に monoid::AffineSum<ZMod<998244353>> を渡す。中身は 16 分木の遅延セグ木 (wide_lazy3 と同じ)。
struct Solver {
  using Z = ZMod<998244353>;
  SegmentTree<monoid::AffineSum<Z>> seg;
  explicit Solver(const vector<i64> &a) : seg(int(a.size()), [&](int i) { return Z(a[i]); }) {}
  void affine(int l, int r, i64 b, i64 c) { seg.apply(l, r, {Z(b), Z(c)}); }
  i64 sum(int l, int r) { return seg.prod(l, r).val(); }
};
