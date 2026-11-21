#pragma once
// NeoLibrary の MaxFlow で 1 から N へ流し、edges() の流量から残余グラフの弧を作って strongly_connected_components (成分の
// 番号はトポロジカル順) で成分に分ける。問いには common.hpp で答える。
#include "common.hpp"
#include "neo/algo/optimize/flow/MaxFlow.hpp"
#include "neo/algo/decide/graph/strongly_connected_components.hpp"

struct Solver {
  int n;
  const vector<array<int, 2>> &roads, &qs;
  Solver(int n, const vector<array<int, 2>> &roads, const vector<array<int, 2>> &qs) : n(n), roads(roads), qs(qs) {}
  vector<int> run() {
    MaxFlow<int> g(n);
    for (auto &[a, b] : roads) g.add_edge(a, b, 1);
    g.flow(0, n - 1);
    vector<array<int, 2>> arcs;
    arcs.reserve(roads.size() * 2);
    for (auto &e : g.edges()) {
      if (e.flow < e.cap) arcs.push_back({e.from, e.to});
      if (e.flow > -e.rev_cap) arcs.push_back({e.to, e.from});
    }
    const StronglyConnectedComponents scc = strongly_connected_components(n, arcs);
    return answer_pairs(n, scc.count, scc.comp, arcs, qs);
  }
};
