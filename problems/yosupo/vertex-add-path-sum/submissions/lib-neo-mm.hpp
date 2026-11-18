#pragma once
#include "pj.hpp"
#include "neo/data_structure/segtree/Sum.hpp"
#include "neo/graph/HeavyLightDecomposition.hpp"

// lib-neo の切り分け。中身は lib-neo と同じで、path から受けた区間を、向きで分岐せずに min と max で渡す。
// lib-neo の l <= r の分岐は、上る区間が 1 頂点 (l == r) かどうかで決まり、乱択の木では読みにくい。
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
    h.path(u, v, [&](int l, int r) { s += seg.prod(min(l, r), max(l, r) + 1); });
    return s;
  }
};
