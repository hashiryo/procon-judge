#pragma once
#include "pj.hpp"
#include "neo/data_structure/segtree/Sum.hpp"
#include "neo/graph/HeavyLightDecomposition.hpp"

// NeoLibrary の HeavyLightDecomposition を辺の列から作り、道を位置の区間に分けて SegmentTree<monoid::Sum<i64>> (8 分木の
// 接頭辞和) の和を足す。区間和の木は init (位置 i の値 = a[to_vertex(i)]) で作り、和は向きによらないので区間を min と max で渡す。
struct Solver {
  HeavyLightDecomposition h;
  SegmentTree<monoid::Sum<i64>> seg;
  Solver(int n, const vector<i64> &a, const vector<array<int, 2>> &edges)
      : h(n, edges), seg(n, [&](int i) { return a[h.to_vertex(i)]; }) {}
  void add(int p, i64 x) { seg.mul(h.to_seq(p), x); }
  i64 path_sum(int u, int v) {
    i64 s = 0;
    h.path(u, v, [&](int l, int r) { s += seg.prod(min(l, r), max(l, r) + 1); });
    return s;
  }
};
