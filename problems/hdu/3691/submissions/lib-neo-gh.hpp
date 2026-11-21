#pragma once
// 全域最小カットの口は NeoLibrary に無いので、NeoLibrary の gomory_hu で Gomory-Hu 木を作り、木の辺の重みの最小を答える
// (全域最小カットは、どこかの 2 頂点の最小カットなので、木のどこかの辺の重みに等しい)。
#include <tuple>
#include "neo/algo/optimize/flow/gomory_hu.hpp"

struct Solver {
  int n;
  const vector<array<i64, 3>> &e;
  Solver(int n, const vector<array<i64, 3>> &e) : n(n), e(e) {}
  i64 run() {
    vector<tuple<int, int, i64>> g;
    g.reserve(e.size());
    for (auto &[u, v, w] : e) g.emplace_back((int)u, (int)v, w);
    const auto t = gomory_hu<i64>(n, g);
    i64 best = -1;
    for (int v = 1; v < n; ++v)
      if (best < 0 || t.weight[v] < best) best = t.weight[v];
    return best < 0 ? 0 : best;
  }
};
