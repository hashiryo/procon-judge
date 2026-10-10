#pragma once
#include "pj.hpp"
#include "mylib/optimization/MaxFlow.hpp"

// Library の PushRelabel で、freeze を true にして前流を作った時点で止めるもの。lib-push-relabel との差が、
// t に届かなかった余りを s へ戻して流れを整える段の費用になる。値だけを求める hlpp 系と同じ仕事の量で比べる。
struct Solver {
  MaxFlow<PushRelabel<i64, 4, true, true>> g;
  int s, t;
  i64 ans = 0;

  Solver(int n, int s, int t, const vector<array<int, 3>> &edges) : g(n), s(s), t(t) {
    for (auto &e : edges) g.add_edge(e[0], e[1], e[2]);
  }

  void run() { ans = g.maxflow(s, t); }

  i64 answer() const { return ans; }
};
