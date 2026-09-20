#pragma once
#include "pj.hpp"
#include "mylib/optimization/NetworkSimplex.hpp"

// 最小費用流に落とす。左右の頂点に供給と需要を 1 ずつ置いて、N^2 本の辺を
// 容量 1 費用 a[i][j] で張る。ネットワーク単体法で解く。
struct Solver {
  using MCF = NetworkSimplex<i64, i64>;

  int n;
  vector<i64> a;
  i64 total = 0;
  vector<int> p;

  Solver(int n, const vector<i64> &a) : n(n), a(a) {}

  void run() {
    MCF g;
    auto left = g.add_vertices(n);
    auto right = g.add_vertices(n);
    for (int i = 0; i < n; ++i) g.add_supply(left[i], 1), g.add_demand(right[i], 1);
    vector<vector<MCF::EdgePtr>> es(n, vector<MCF::EdgePtr>(n));
    for (int i = 0; i < n; ++i)
      for (int j = 0; j < n; ++j)
        es[i][j] = g.add_edge(left[i], right[j], 0, 1, a[(size_t)i * n + j]);
    g.b_flow();
    total = g.get_result_value();
    p.assign(n, -1);
    for (int i = 0; i < n; ++i)
      for (int j = 0; j < n; ++j)
        if (es[i][j].flow()) p[i] = j;
  }

  i64 cost() const { return total; }

  const vector<int> &assignment() const { return p; }
};
