#pragma once
#include "pj.hpp"
#include "mylib/graph/Graph.hpp"
#include "mylib/graph/HeavyLightDecomposition.hpp"

// HLD。木を 1 本の列に潰しておいて、重い辺を辿って登る。前処理は O(N) で、
// 1 クエリは O(log N)。
struct Solver {
  HeavyLightDecomposition hld;

  Solver(int n, const vector<int> &par) {
    Graph g(n);
    for (int i = 1; i < n; ++i) g.add_edge(par[i], i);
    hld = HeavyLightDecomposition(g, 0);
  }

  int lca(int u, int v) { return hld.lca(u, v); }
};
