#pragma once
#include "pj.hpp"
#include "mylib/optimization/NetworkSimplex.hpp"

// Library の NetworkSimplex。block search で入る辺を選ぶネットワーク単体法。
struct Solver {
  using MCF = NetworkSimplex<i64, i64>;
  int n;
  MCF g;
  vector<MCF::EdgePtr> es;
  bool ok = false;
  __int128 total = 0;
  vector<i64> pot, fl;

  Solver(int n, const vector<i64> &b, const vector<array<i64, 5>> &edges) : n(n), g(n) {
    for (int i = 0; i < n; ++i) g.add_supply(i, b[i]);
    es.reserve(edges.size());
    for (auto &e : edges) es.push_back(g.add_edge((int)e[0], (int)e[1], e[2], e[3], e[4]));
  }

  void run() {
    ok = g.b_flow();
    if (!ok) return;
    total = g.get_result_value<__int128>();
    pot.resize(n);
    for (int i = 0; i < n; ++i) pot[i] = g.get_potential(i);
    fl.resize(es.size());
    for (size_t i = 0; i < es.size(); ++i) fl[i] = es[i].flow();
  }

  bool feasible() const { return ok; }
  __int128 cost() const { return total; }
  const vector<i64> &potential() const { return pot; }
  const vector<i64> &flow() const { return fl; }
};
