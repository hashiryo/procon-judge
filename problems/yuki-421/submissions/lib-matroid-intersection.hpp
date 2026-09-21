#pragma once
#include "common.hpp"
#include "mylib/optimization/matroid_intersection.hpp"

// 2 部マッチングを、辺の集合の上の 2 つの分割マトロイド (左端点ごと 1 本、右端点
// ごと 1 本) の共通独立集合として取る。汎用の枠組みなので、専用の増加路より重い。
struct Solver {
  vector<string> grid;
  i64 ans = 0;

  explicit Solver(const vector<string> &grid) : grid(grid) {}

  void run() {
    Chocolate c = parse(grid);
    const int e = (int)c.edges.size();
    vector<vector<int>> partl(c.cells), partr(c.cells);
    for (int i = 0; i < e; ++i) partl[c.edges[i][0]].push_back(i), partr[c.edges[i][1]].push_back(i);
    PartitionMatroid m1(e, partl, vector(c.cells, 1)), m2(e, partr, vector(c.cells, 1));
    ans = happiness(c, (int)matroid_intersection(e, m1, m2).size());
  }

  i64 answer() const { return ans; }
};
