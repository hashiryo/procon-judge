#pragma once
#include "pj.hpp"
#include "neo/data_structure/segtree/Sum.hpp"
#include "neo/graph/HeavyLightDecomposition.hpp"

// NeoLibrary の HeavyLightDecomposition を親の列から作り、部分木の位置の区間に SegmentTree<monoid::Sum<i64>> (8 分木の接頭辞和)
// を載せる。
struct Solver {
  HeavyLightDecomposition h;
  SegmentTree<monoid::Sum<i64>> seg;
  static vector<i64> by_pos(const HeavyLightDecomposition &h, const vector<i64> &a) {
    vector<i64> v(a.size());
    for (int u = 0; u < (int)a.size(); ++u) v[h.to_seq(u)] = a[u];
    return v;
  }
  Solver(int, const vector<i64> &a, const vector<int> &par) : h(par), seg(by_pos(h, a)) {}
  void add(int u, i64 x) { seg.mul(h.to_seq(u), x); }
  i64 subtree_sum(int u) {
    auto [l, r] = h.subtree(u);
    return seg.prod(l, r);
  }
};
