#pragma once
#include "pj.hpp"
#include "neo/base/ds/range/segtree/Sum.hpp"
#include "neo/base/ds/tree/LCA.hpp"

// NeoLibrary で xor_euler_rmq_wide と同じ形を書いたもの。根から各頂点までの和 S を、行きがけ順の位置の差分の列として
// SegmentTree<monoid::Sum<i64>> (8 分木の接頭辞和) に持ち、頂点 k への加算は部分木の区間の両端の 2 点に足す。u と v の
// 道の和は S(u) + S(v) - 2 S(w) + a_w で、w は LCA (O(1))。
struct Solver {
  HeavyLightDecomposition h;
  LCA lca;
  vector<i64> val;  // 頂点ごとの値
  SegmentTree<monoid::Sum<i64>> seg;
  static vector<i64> diff(const HeavyLightDecomposition &h, const vector<i64> &a) {
    const int n = (int)a.size();
    vector<i64> s(n), d(n);
    for (int x = 0; x < n; ++x) {
      const int v = h.to_vertex(x), p = h.parent(v);
      s[x] = (p < 0 ? 0 : s[h.to_seq(p)]) + a[v];
    }
    for (int x = 0; x < n; ++x) d[x] = s[x] - (x ? s[x - 1] : 0);
    return d;
  }
  Solver(int n, const vector<i64> &a, const vector<array<int, 2>> &edges) : h(n, edges), lca(h), val(a), seg(diff(h, a)) {}
  void add(int p, i64 x) {
    auto [l, r] = h.subtree(p);
    val[p] += x, seg.mul(l, x);
    if (r < (int)h.size()) seg.mul(r, -x);
  }
  i64 path_sum(int u, int v) {
    const int w = lca(u, v);
    return seg.prod(0, h.to_seq(u) + 1) + seg.prod(0, h.to_seq(v) + 1) - 2 * seg.prod(0, h.to_seq(w) + 1) + val[w];
  }
};
