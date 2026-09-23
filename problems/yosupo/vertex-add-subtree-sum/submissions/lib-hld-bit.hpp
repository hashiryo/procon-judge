#pragma once
#include "pj.hpp"
#include "mylib/graph/Graph.hpp"
#include "mylib/graph/HeavyLightDecomposition.hpp"
#include "mylib/data_structure/BinaryIndexedTree.hpp"

// 木を行きがけ順の列に潰すと、部分木が区間になる。あとは BIT の区間和。
// 構築は O(N)、1 クエリは O(log N)。
struct Solver {
  HeavyLightDecomposition hld;
  BinaryIndexedTree<i64> bit;

  static HeavyLightDecomposition make_hld(int n, const vector<int> &par) {
    Graph g(n);
    for (int i = 1; i < n; ++i) g.add_edge(par[i], i);
    return HeavyLightDecomposition(g, 0);
  }

  static vector<i64> to_seq_order(const HeavyLightDecomposition &h,
                                  const vector<i64> &a) {
    vector<i64> v(a.size());
    for (int u = (int)a.size(); u--;) v[h.to_seq(u)] = a[u];
    return v;
  }

  Solver(int n, const vector<i64> &a, const vector<int> &par)
      : hld(make_hld(n, par)), bit(to_seq_order(hld, a)) {}

  void add(int u, i64 x) { bit.add(hld.to_seq(u), x); }

  i64 subtree_sum(int u) {
    auto [l, r] = hld.subtree(u);
    return bit.sum(l, r);
  }
};
