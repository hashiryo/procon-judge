#pragma once
#include "common.hpp"
#include "neo/combinatorial_optimization/matroid_intersection.hpp"
#include "neo/combinatorial_optimization/matroids.hpp"

// 2 部マッチングを、辺の集合の上の 2 つの分割マトロイド (左端点ごと 1 本、右端点ごと 1 本) の共通独立集合として取る。
// NeoLibrary の matroid_intersection と PartitionMatroid を使う。
struct Solver {
  vector<string> grid;
  i64 ans = 0;

  explicit Solver(const vector<string> &grid) : grid(grid) {}

  void run() {
    Chocolate c = parse(grid);
    const int e = (int)c.edges.size();
    vector<int> pl(e), pr(e);
    for (int i = 0; i < e; ++i) pl[i] = c.edges[i][0], pr[i] = c.edges[i][1];
    const vector<int> one(c.cells, 1);
    ans = happiness(c, (int)matroid_intersection(PartitionMatroid(pl, one), PartitionMatroid(pr, one)).size());
  }

  i64 answer() const { return ans; }
};
