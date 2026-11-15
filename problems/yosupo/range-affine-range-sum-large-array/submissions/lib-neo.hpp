#pragma once
#include "pj.hpp"
#include "neo/algebra/ZMod.hpp"
#include "neo/data_structure/DynamicSegmentTree.hpp"
#include "neo/data_structure/segtree/Affine.hpp"

// NeoLibrary の DynamicSegmentTree に monoid::AffineSum<ZMod<998244353>> を渡す。作用があるので中身は、子の無い節を
// 「その下は ti() のまま」とする動的木 (dyn_uniform と同じく、子ができるのは区間の端の道だけ)。
struct Solver {
  using Z = ZMod<998244353>;
  DynamicSegmentTree<monoid::AffineSum<Z>> seg;
  explicit Solver(int n) : seg(n) {}
  void affine(int l, int r, i64 b, i64 c) { seg.apply(l, r, {Z(b), Z(c)}); }
  i64 sum(int l, int r) { return seg.prod(l, r).val(); }
};
