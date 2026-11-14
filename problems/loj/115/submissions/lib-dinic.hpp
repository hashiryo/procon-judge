#pragma once
// 今の Library の MaxFlowLowerBound (Dinic)。s と t を持たない流れを聞く口が無いので、辺の無い頂点 2 つを s と t にして
// maxflow を呼ぶ (中で足す t から s への辺には流れない)。辺ごとの流量は EdgePtr::flow で読む。
#include "pj.hpp"
#include "mylib/optimization/MaxFlow.hpp"
struct Solver {
  using MF = MaxFlowLowerBound<Dinic<i64>>;
  MF g;
  vector<MF::EdgePtr> ep;
  int s, t;
  bool ok = false;
  vector<i64> f;
  Solver(int n, const vector<array<int, 4>> &edges) : g(n) {
    s = g.add_vertex(), t = g.add_vertex();
    ep.reserve(edges.size());
    for (auto &e : edges) ep.push_back(g.add_edge(e[0], e[1], e[2], e[3]));
  }
  void run() {
    if (!(ok = g.maxflow(s, t) >= 0)) return;
    f.resize(ep.size());
    for (size_t i = 0; i < ep.size(); ++i) f[i] = ep[i].flow();
  }
  bool feasible() const { return ok; }
  const vector<i64> &flows() const { return f; }
};
