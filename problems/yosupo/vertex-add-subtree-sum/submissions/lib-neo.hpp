#pragma once
#include "pj.hpp"
#include "neo/base/ds/range/segtree/Sum.hpp"
#include "neo/base/ds/tree/HeavyLightDecomposition.hpp"

// NeoLibrary の HeavyLightDecomposition を親の列から作り、部分木の位置の区間に SegmentTree<monoid::Sum<i64>> (8 分木の接頭辞和)
// を載せる。区間和の木は、位置ごとの値の列を作らずに init (位置 i の値 = a[to_vertex(i)]) で作る。
struct Solver {
  HeavyLightDecomposition h;
  SegmentTree<monoid::Sum<i64>> seg;
  Solver(int n, const vector<i64> &a, const vector<int> &par) : h(par), seg(n, [&](int i) { return a[h.to_vertex(i)]; }) {}
  void add(int u, i64 x) { seg.mul(h.to_seq(u), x); }
  i64 subtree_sum(int u) {
    auto [l, r] = h.subtree(u);
    return seg.prod(l, r);
  }
};
