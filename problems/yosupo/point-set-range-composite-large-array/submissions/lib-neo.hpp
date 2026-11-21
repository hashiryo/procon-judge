#pragma once
#include "pj.hpp"
#include "neo/base/core/algebra/ZMod.hpp"
#include "neo/base/ds/range/DynamicSegmentTree.hpp"
#include "neo/base/ds/range/segtree/Composite.hpp"

// NeoLibrary の DynamicSegmentTree に monoid::Composite<ZMod<998244353>> を渡す。モノイドだけなので中身はパトリシア木
// (patricia と同じ形で、区間の積は x に当てずに 1 次関数を畳む)。
struct Solver {
  using Z = ZMod<998244353>;
  using CM = monoid::Composite<Z>;
  DynamicSegmentTree<CM> seg;
  explicit Solver(int n) : seg(n) {}
  void set(int p, i64 c, i64 d) { seg.set(p, {Z(c), Z(d)}); }
  i64 query(int l, int r, i64 x) { return CM::eval(seg.prod(l, r), Z(x)).val(); }
};
