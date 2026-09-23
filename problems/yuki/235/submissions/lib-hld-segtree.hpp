#pragma once
#include "common.hpp"
#include "mylib/graph/Graph.hpp"
#include "mylib/graph/HeavyLightDecomposition.hpp"
#include "mylib/data_structure/SegmentTree.hpp"

// HLD で木を列に潰すと、道が O(log N) 本の区間になる。区間への作用と区間の和は
// 遅延セグメント木。和は可換なので、反転した列は要らない。
struct Solver {
  HeavyLightDecomposition hld;
  SegmentTree<Inflation> seg;

  static HeavyLightDecomposition make_hld(int n, const vector<array<int, 2>> &edges) {
    Graph g(n);
    for (auto &e : edges) g.add_edge(e[0], e[1]);
    return HeavyLightDecomposition(g, 0);
  }

  static vector<Inflation::T> to_seq_order(const HeavyLightDecomposition &h,
                                           const vector<i64> &s, const vector<i64> &c) {
    vector<Inflation::T> v(s.size());
    for (int u = (int)s.size(); u--;) v[h.to_seq(u)] = {Mint(s[u]), Mint(c[u])};
    return v;
  }

  Solver(int n, const vector<i64> &s, const vector<i64> &c,
         const vector<array<int, 2>> &edges)
      : hld(make_hld(n, edges)), seg(to_seq_order(hld, s, c)) {}

  void parade(int x, int y, i64 z) {
    const Mint w(z);
    for (auto [a, b] : hld.path(x, y))
      a < b ? seg.apply(a, b + 1, w) : seg.apply(b, a + 1, w);
  }

  i64 travel(int x, int y) {
    Mint acc;
    for (auto [a, b] : hld.path(x, y))
      acc += (a < b ? seg.prod(a, b + 1) : seg.prod(b, a + 1)).s;
    return acc.val();
  }
};
