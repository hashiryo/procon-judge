#pragma once
#include "pj.hpp"
#include "neo/data_structure/segtree/Affine.hpp"

// NeoLibrary の SegmentTree に monoid::Affine<ZMod<998244353>> を渡す。中身は 16 分木の双対セグ木 (wide_dual2_pf と同じ)。
struct Solver {
  using Z = ZMod<998244353>;
  SegmentTree<monoid::Affine<Z>> seg;
  explicit Solver(const vector<i64> &a) : seg(int(a.size()), [&](int i) { return Z(a[i]); }) {}
  void affine(int l, int r, i64 b, i64 c) { seg.apply(l, r, {Z(b), Z(c)}); }
  i64 get(int i) { return seg.get(i).val(); }
};
