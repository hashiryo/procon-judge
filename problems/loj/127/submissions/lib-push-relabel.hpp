#pragma once
#include "pj.hpp"
#include "mylib/optimization/MaxFlow.hpp"

// Library の PushRelabel。highest label の順に流し、gap と global relabel を使う。
// 前流を作ったあと、t に届かなかった余りを s へ戻して流れを整える。
struct Solver {
  MaxFlow<PushRelabel<i64>> g;
  int s, t;
  i64 ans = 0;

  Solver(int n, int s, int t, const vector<array<int, 3>> &edges) : g(n), s(s), t(t) {
    for (auto &e : edges) g.add_edge(e[0], e[1], e[2]);
  }

  void run() { ans = g.maxflow(s, t); }

  i64 answer() const { return ans; }
};
