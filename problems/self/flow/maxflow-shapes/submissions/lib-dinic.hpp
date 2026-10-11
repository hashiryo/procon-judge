#pragma once
#include "pj.hpp"
#include "mylib/optimization/MaxFlow.hpp"

// Library の Dinic。容量と逆向きの容量が等しい辺は bidir で 1 組の弧にし、違う辺は向きごとに辺を足す。
struct Solver {
  MaxFlow<Dinic<i64>> g;
  int s, t;
  i64 ans = 0;

  Solver(int n, int s, int t, const vector<array<int, 4>> &edges) : g(n), s(s), t(t) {
    for (auto &e : edges) {
      if (e[2] == e[3]) g.add_edge(e[0], e[1], e[2], true);
      else {
        g.add_edge(e[0], e[1], e[2]);
        if (e[3] > 0) g.add_edge(e[1], e[0], e[3]);
      }
    }
  }

  void run() { ans = g.maxflow(s, t); }

  i64 answer() const { return ans; }
};
