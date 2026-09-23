#pragma once
#include "pj.hpp"
#include "mylib/graph/Graph.hpp"
#include "mylib/graph/HeavyLightDecomposition.hpp"
#include "mylib/data_structure/BinaryIndexedTree.hpp"

// HLD で木を列に潰すと、道が O(log N) 本の区間になる。区間和は BIT。
// 構築は O(N)、1 クエリは O(log^2 N)。
struct Solver {
  HeavyLightDecomposition hld;
  BinaryIndexedTree<i64> bit;

  static HeavyLightDecomposition make_hld(int n,
                                          const vector<array<int, 2>> &edges) {
    Graph g(n);
    for (auto &e : edges) g.add_edge(e[0], e[1]);
    return HeavyLightDecomposition(g, 0);
  }

  static vector<i64> to_seq_order(const HeavyLightDecomposition &h,
                                  const vector<i64> &a) {
    vector<i64> v(a.size());
    for (int u = (int)a.size(); u--;) v[h.to_seq(u)] = a[u];
    return v;
  }

  Solver(int n, const vector<i64> &a, const vector<array<int, 2>> &edges)
      : hld(make_hld(n, edges)), bit(to_seq_order(hld, a)) {}

  void add(int p, i64 x) { bit.add(hld.to_seq(p), x); }

  i64 path_sum(int u, int v) {
    i64 s = 0;
    for (auto [x, y] : hld.path(u, v))
      s += x < y ? bit.sum(x, y + 1) : bit.sum(y, x + 1);
    return s;
  }
};
