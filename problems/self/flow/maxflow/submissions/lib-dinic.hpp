#pragma once
#include "pj.hpp"
#include "mylib/optimization/MaxFlow.hpp"

// Library の Dinic。s からの BFS で層を作り、t から s へ層を逆に辿る DFS で流す。
struct Solver {
  MaxFlow<Dinic<i64>> g;
  int s, t;
  i64 ans = 0;

  Solver(int n, int s, int t, const vector<array<int, 3>> &edges) : g(n), s(s), t(t) {
    for (auto &e : edges) g.add_edge(e[0], e[1], e[2]);
  }

  void run() { ans = g.maxflow(s, t); }

  i64 answer() const { return ans; }
};
