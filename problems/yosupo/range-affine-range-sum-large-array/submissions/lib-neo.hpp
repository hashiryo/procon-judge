#pragma once
#include "pj.hpp"
#include "neo/base/core/algebra/ZMod.hpp"
#include "neo/base/ds/range/DynamicSegmentTree.hpp"
#include "neo/base/ds/range/segtree/Affine.hpp"

// NeoLibrary の DynamicSegmentTree に monoid::AffineSum<ZMod<998244353>> を渡す。この標準の構造体のときは中身が
// dyn_uniform と同じ核 (子の無い節は値のすべて等しい区間、値は Montgomery、区間の長さの表) に替わる (NeoLibrary 710b475)。
struct Solver {
  using Z = ZMod<998244353>;
  DynamicSegmentTree<monoid::AffineSum<Z>> seg;
  explicit Solver(int n) : seg(n) {}
  void affine(int l, int r, i64 b, i64 c) { seg.apply(l, r, {Z(b), Z(c)}); }
  i64 sum(int l, int r) { return seg.prod(l, r).val(); }
};
