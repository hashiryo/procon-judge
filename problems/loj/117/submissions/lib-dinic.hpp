#pragma once
// 今の Library の MaxFlow (Dinic) で組む、よく知られた最小流の形。今の Library の MaxFlowLowerBound には最小流の口が
// 無いので、帰着を手で書く。下限を先に流した過不足を超頂点 S と T への辺で埋め、まず t から s への辺なしで S から T へ
// 流し、そのあと t から s へ容量の無限大の辺を足して続けて流す。全部流れれば、その辺の流量が最小流になる。
#include "pj.hpp"
#include "mylib/optimization/MaxFlow.hpp"
struct Solver {
  using MF = MaxFlow<Dinic<i64>>;
  int n, s, t;
  const vector<array<int, 4>> &edges;
  i64 ans = -1;
  Solver(int n, int s, int t, const vector<array<int, 4>> &edges) : n(n), s(s), t(t), edges(edges) {}
  void run() {
    MF g(n + 2);
    const int S = n, T = n + 1;
    vector<i64> ex(n);
    for (auto &e : edges) {
      if (e[3] > e[2]) g.add_edge(e[0], e[1], e[3] - e[2]);
      ex[e[0]] -= e[2], ex[e[1]] += e[2];
    }
    i64 need = 0;
    for (int v = 0; v < n; ++v)
      if (ex[v] > 0) g.add_edge(S, v, ex[v]), need += ex[v];
      else if (ex[v] < 0) g.add_edge(v, T, -ex[v]);
    i64 got = g.maxflow(S, T);
    auto aux = g.add_edge(t, s, numeric_limits<i64>::max() / 2);
    got += g.maxflow(S, T);
    ans = got == need ? aux.flow() : -1;
  }
  i64 answer() const { return ans; }
};
