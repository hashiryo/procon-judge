#pragma once
#include "pj.hpp"
#include "mylib/optimization/MaxFlow.hpp"

// レベルグラフを作って行き止まりを刈りながら増加路を流す。素直な実装で、疎なグラフでは速い。
struct Solver {
  int n;
  vector<array<i64, 3>> edges;
  i64 ans = 0;

  Solver(int n, const vector<array<i64, 3>> &edges) : n(n), edges(edges) {}

  void run() {
    MaxFlow<Dinic<i64>> g(n);
    for (auto &e : edges) g.add_edge((int)e[0], (int)e[1], e[2]);
    ans = g.maxflow(0, n - 1);
  }

  i64 answer() const { return ans; }
};
