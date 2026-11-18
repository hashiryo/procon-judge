#pragma once
// 今の Library の MaxFlow (Dinic) で 1 から N へ流し、残余グラフ (残りの容量のある向きの弧) を Graph に組んで
// StronglyConnectedComponents (成分の番号はトポロジカル順) で成分に分ける。問いには common.hpp で答える。
#include "common.hpp"
#include "mylib/graph/StronglyConnectedComponents.hpp"
#include "mylib/optimization/MaxFlow.hpp"

struct Solver {
  int n;
  const vector<array<int, 2>> &roads, &qs;
  Solver(int n, const vector<array<int, 2>> &roads, const vector<array<int, 2>> &qs) : n(n), roads(roads), qs(qs) {}
  vector<int> run() {
    using MF = MaxFlow<Dinic<int>>;
    MF g(n);
    vector<MF::EdgePtr> es;
    es.reserve(roads.size());
    for (auto &[a, b] : roads) es.push_back(g.add_edge(a, b, 1));
    g.maxflow(0, n - 1);
    Graph r(n);
    vector<array<int, 2>> arcs;
    for (auto &e : es) {
      if (e.flow() < e.cap()) r.add_edge(e.src(), e.dst()), arcs.push_back({e.src(), e.dst()});
      if (e.flow() > 0) r.add_edge(e.dst(), e.src()), arcs.push_back({e.dst(), e.src()});
    }
    StronglyConnectedComponents scc(r);
    vector<int> comp(n);
    for (int v = 0; v < n; ++v) comp[v] = scc(v);
    return answer_pairs(n, scc.size(), comp, arcs, qs);
  }
};
