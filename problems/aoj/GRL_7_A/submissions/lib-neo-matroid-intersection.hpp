#pragma once
#include "pj.hpp"
#include "neo/combinatorial_optimization/matroid_intersection.hpp"
#include "neo/combinatorial_optimization/matroids.hpp"

// マトロイド交叉に落とす。「左の各頂点から出る辺が高々 1 本」と「右の各頂点へ入る辺が高々 1 本」がどちらも分割マトロイド
// で、その共通部分の最大がマッチングになる。NeoLibrary の matroid_intersection と PartitionMatroid を使う。
struct Solver {
  int x, y;
  vector<array<int, 2>> edges;
  i64 ans = 0;

  Solver(int x, int y, const vector<array<int, 2>> &edges) : x(x), y(y), edges(edges) {}

  void run() {
    const int m = (int)edges.size();
    vector<int> pl(m), pr(m);
    for (int i = 0; i < m; ++i) pl[i] = edges[i][0], pr[i] = edges[i][1];
    ans = (i64)matroid_intersection(PartitionMatroid(pl, vector<int>(x, 1)), PartitionMatroid(pr, vector<int>(y, 1))).size();
  }

  i64 answer() const { return ans; }
};
