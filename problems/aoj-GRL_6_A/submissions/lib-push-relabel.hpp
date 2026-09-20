#pragma once
#include "pj.hpp"
#include "mylib/optimization/MaxFlow.hpp"

// 頂点に高さを付けて余剰を押し出す。増加路を探さないので、密なグラフや流量の大きいグラフで強い。
struct Solver {
  int n;
  vector<array<i64, 3>> edges;
  i64 ans = 0;

  Solver(int n, const vector<array<i64, 3>> &edges) : n(n), edges(edges) {}

  void run() {
    MaxFlow<PushRelabel<i64>> g(n);
    for (auto &e : edges) g.add_edge((int)e[0], (int)e[1], e[2]);
    ans = g.maxflow(0, n - 1);
  }

  i64 answer() const { return ans; }
};
