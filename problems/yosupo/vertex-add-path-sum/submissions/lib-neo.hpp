#pragma once
#include "pj.hpp"
#include "neo/data_structure/segtree/Sum.hpp"
#include "neo/graph/HeavyLightDecomposition.hpp"

// NeoLibrary の HeavyLightDecomposition を辺の列から作り、道を位置の区間に分けて SegmentTree<monoid::Sum<i64>> (8 分木の
// 接頭辞和) の和を足す。
struct Solver {
  HeavyLightDecomposition h;
  SegmentTree<monoid::Sum<i64>> seg;
  static vector<i64> by_pos(const HeavyLightDecomposition &h, const vector<i64> &a) {
    vector<i64> v(a.size());
    for (int u = 0; u < (int)a.size(); ++u) v[h.to_seq(u)] = a[u];
    return v;
  }
  Solver(int n, const vector<i64> &a, const vector<array<int, 2>> &edges) : h(n, edges), seg(by_pos(h, a)) {}
  void add(int p, i64 x) { seg.mul(h.to_seq(p), x); }
  i64 path_sum(int u, int v) {
    i64 s = 0;
    h.path(u, v, [&](int l, int r) { s += l <= r ? seg.prod(l, r + 1) : seg.prod(r, l + 1); });
    return s;
  }
};
