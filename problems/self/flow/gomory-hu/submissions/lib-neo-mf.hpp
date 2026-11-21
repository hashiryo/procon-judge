#pragma once
// NeoLibrary の MaxFlow (neo/algo/optimize/flow/MaxFlow.hpp) を組ごとに作り直して、Gusfield の方法を書く。
// algo-notes の network-flow-api.md の案 (MaxFlow を n - 1 回使う薄い層) をそのまま書いた形。無向辺は rev_cap に同じ容量を
// 渡し、最小カットは min_cut(s)。手順は reference.hpp と同じ。
#include "pj.hpp"
#include "neo/algo/optimize/flow/MaxFlow.hpp"
struct Solver {
  int n;
  const vector<array<i64, 3>> &edges;
  vector<array<i64, 3>> res;
  Solver(int n, const vector<array<i64, 3>> &edges) : n(n), edges(edges) {}
  void run() {
    vector<int> p(n, 0);
    vector<i64> fl(n, 0);
    for (int s = 1; s < n; ++s) {
      const int t = p[s];
      MaxFlow<i64> g(n);
      for (auto &e : edges)
        if (e[0] != e[1]) g.add_edge((int)e[0], (int)e[1], e[2], e[2]);
      const i64 f = g.flow(s, t);
      const vector<bool> X = g.min_cut(s);
      fl[s] = f;
      for (int i = 0; i < n; ++i)
        if (i != s && X[i] && p[i] == t) p[i] = s;
      if (t != 0 && X[p[t]]) p[s] = p[t], p[t] = s, fl[s] = fl[t], fl[t] = f;
    }
    res.clear();
    for (int v = 1; v < n; ++v) res.push_back({v, p[v], fl[v]});
  }
  const vector<array<i64, 3>> &tree() const { return res; }
};
