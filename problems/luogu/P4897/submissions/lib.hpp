#pragma once
// 今の Library には Gomory-Hu が無いので、今の Library の MaxFlow (Dinic) で Gusfield の方法を書く。組ごとに網を作り直し、
// 無向辺は add_edge の bidir で張る。最小カットは mincut(s) (残余グラフで s から届く頂点)。手順は reference.hpp と同じ。
#include "pj.hpp"
#include "mylib/optimization/MaxFlow.hpp"
struct Solver {
  using MF = MaxFlow<Dinic<i64>>;
  int n;
  const vector<array<i64, 3>> &edges;
  vector<array<i64, 3>> res;
  Solver(int n, const vector<array<i64, 3>> &edges) : n(n), edges(edges) {}
  void run() {
    vector<int> p(n, 0);
    vector<i64> fl(n, 0);
    for (int s = 1; s < n; ++s) {
      const int t = p[s];
      MF g(n);
      for (auto &e : edges)
        if (e[0] != e[1]) g.add_edge((int)e[0], (int)e[1], e[2], true);
      const i64 f = g.maxflow(s, t);
      const vector<bool> X = g.mincut(s);
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
