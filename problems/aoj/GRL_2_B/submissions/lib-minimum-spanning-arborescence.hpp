#pragma once
#include "pj.hpp"
#include "mylib/graph/Graph.hpp"
#include "mylib/graph/minimum_spanning_aborescence.hpp"

// 最小全域有向木の専用実装 (Chu-Liu/Edmonds)。各頂点の最小入辺を取って、
// 閉路ができたら縮約する。
struct Solver {
  int n, root;
  vector<array<i64, 3>> edges;
  i64 ans = 0;

  Solver(int n, int root, const vector<array<i64, 3>> &edges)
      : n(n), root(root), edges(edges) {}

  void run() {
    Graph g(n);
    vector<i64> w;
    w.reserve(edges.size());
    for (auto &e : edges) g.add_edge((int)e[0], (int)e[1]), w.push_back(e[2]);
    ans = minimum_spanning_aborescence(g, w, root).first;
  }

  i64 answer() const { return ans; }
};
