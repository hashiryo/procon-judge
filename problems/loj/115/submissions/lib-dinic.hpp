#pragma once
// 今の Library の MaxFlow (Dinic) で組む、よく知られた形。今の Library の MaxFlowLowerBound は辺の流量を読む EdgePtr::flow
// が組めない (const の flow が const でない edge を呼ぶ) ので、帰着を手で書く。下限を先に流した過不足を超頂点 S と T への
// 辺で埋め、S から T へ全部流れれば、下限に MaxFlow の辺の流量を足したものが流れになる。
#include "pj.hpp"
#include "mylib/optimization/MaxFlow.hpp"
struct Solver {
  using MF = MaxFlow<Dinic<i64>>;
  int n;
  const vector<array<int, 4>> &edges;
  bool ok = false;
  vector<i64> f;
  Solver(int n, const vector<array<int, 4>> &edges) : n(n), edges(edges) {}
  void run() {
    MF g(n + 2);
    const int S = n, T = n + 1;
    vector<MF::EdgePtr> ep;
    ep.reserve(edges.size());
    vector<i64> ex(n);
    for (auto &e : edges) ep.push_back(g.add_edge(e[0], e[1], e[3] - e[2])), ex[e[0]] -= e[2], ex[e[1]] += e[2];
    i64 need = 0;
    for (int v = 0; v < n; ++v)
      if (ex[v] > 0) g.add_edge(S, v, ex[v]), need += ex[v];
      else if (ex[v] < 0) g.add_edge(v, T, -ex[v]);
    if (!(ok = g.maxflow(S, T) == need)) return;
    f.resize(edges.size());
    for (size_t i = 0; i < edges.size(); ++i) f[i] = edges[i][2] + ep[i].flow();
  }
  bool feasible() const { return ok; }
  const vector<i64> &flows() const { return f; }
};
