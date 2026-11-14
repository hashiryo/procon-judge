#pragma once
// 今の Library の MaxFlowLowerBound (Dinic)。
#include "pj.hpp"
#include "mylib/optimization/MaxFlow.hpp"
struct Solver {
  MaxFlowLowerBound<Dinic<i64>> g;
  int s, t;
  i64 ans = -1;
  Solver(int n, int s, int t, const vector<array<int, 4>> &edges) : g(n), s(s), t(t) {
    for (auto &e : edges) g.add_edge(e[0], e[1], e[2], e[3]);
  }
  void run() { ans = g.maxflow(s, t); }
  i64 answer() const { return ans; }
};
