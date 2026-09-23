#pragma once
#include "pj.hpp"
#include "mylib/optimization/matroid_intersection.hpp"

// マトロイド交叉に落とす。「左の各頂点から出る辺が高々 1 本」と「右の各頂点
// へ入る辺が高々 1 本」がどちらも分割マトロイドで、その共通部分の最大が
// マッチングになる。
struct Solver {
  int x, y;
  vector<array<int, 2>> edges;
  i64 ans = 0;

  Solver(int x, int y, const vector<array<int, 2>> &edges)
      : x(x), y(y), edges(edges) {}

  void run() {
    const int m = (int)edges.size();
    vector<vector<int>> left(x), right(y);
    for (int i = 0; i < m; ++i)
      left[edges[i][0]].push_back(i), right[edges[i][1]].push_back(i);
    PartitionMatroid m1(m, left, vector<int>(x, 1));
    PartitionMatroid m2(m, right, vector<int>(y, 1));
    ans = (i64)matroid_intersection(m, m1, m2).size();
  }

  i64 answer() const { return ans; }
};
