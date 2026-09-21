#pragma once
#include "common.hpp"
#include "mylib/optimization/matroid_intersection.hpp"

// 重みなしの問題をあえて重み付きマトロイド交差で解く。全部の辺に重み 1 を付けて
// 最大化し、大きさごとの最適解の列から最大のものを読む。
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
    int x = (int)weighted_matroid_intersection<MAXIMIZE>(e, m1, m2, vector(e, 1)).size() - 1;
    ans = happiness(c, x);
  }

  i64 answer() const { return ans; }
};
