#pragma once
// NeoLibrary の gomory_hu (neo/combinatorial_optimization/gomory_hu.hpp) で木を作り、親と重みの列を木の辺の列にする。
#include "pj.hpp"
#include "neo/combinatorial_optimization/gomory_hu.hpp"
struct Solver {
  int n;
  const vector<array<i64, 3>> &edges;
  vector<array<i64, 3>> res;
  Solver(int n, const vector<array<i64, 3>> &edges) : n(n), edges(edges) {}
  void run() {
    vector<tuple<int, int, long long>> es(edges.size());
    for (size_t i = 0; i < edges.size(); ++i) es[i] = {(int)edges[i][0], (int)edges[i][1], edges[i][2]};
    auto g = gomory_hu(n, es);
    res.resize(max(n - 1, 0));
    for (int v = 1; v < n; ++v) res[v - 1] = {v, g.parent[v], g.weight[v]};
  }
  const vector<array<i64, 3>> &tree() const { return res; }
};
