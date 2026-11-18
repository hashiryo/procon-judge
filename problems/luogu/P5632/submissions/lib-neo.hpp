#pragma once
// NeoLibrary の global_min_cut で答える。
#include <tuple>
#include "neo/combinatorial_optimization/global_min_cut.hpp"

struct Solver {
  int n;
  const vector<array<i64, 3>> &e;
  Solver(int n, const vector<array<i64, 3>> &e) : n(n), e(e) {}
  i64 run() {
    vector<tuple<int, int, i64>> g;
    g.reserve(e.size());
    for (auto &[u, v, w] : e) g.emplace_back((int)u, (int)v, w);
    return global_min_cut<i64>(n, g);
  }
};
