#pragma once
#include "pj.hpp"
#include "neo/combinatorial_optimization/matroid_intersection.hpp"
#include "neo/combinatorial_optimization/matroids.hpp"

// 重み付きマトロイド交叉に落とす。辺の集合について「閉路を作らない」条件がグラフ的マトロイド、「各頂点への入辺が高々
// 1 本 (根へは 0 本)」が分割マトロイドで、その共通部分の要素数 n - 1 の最小重みが求める有向木になる。NeoLibrary の
// weighted_matroid_intersection は重みの和の最大を求めるので、重みの符号を反転して渡し、要素数 n - 1 で打ち切る。
struct Solver {
  int n, root;
  vector<array<i64, 3>> edges;
  i64 ans = 0;

  Solver(int n, int root, const vector<array<i64, 3>> &edges) : n(n), root(root), edges(edges) {}

  void run() {
    const int m = (int)edges.size();
    vector<array<int, 2>> es(m);
    vector<int> part(m);
    vector<i64> w(m);
    for (int i = 0; i < m; ++i) es[i] = {(int)edges[i][0], (int)edges[i][1]}, part[i] = (int)edges[i][1], w[i] = -edges[i][2];
    vector<int> cap(n, 1);
    cap[root] = 0;  // 根には入らない
    auto [best, I] = weighted_matroid_intersection(GraphicMatroid(n, es), PartitionMatroid(part, cap), w, n - 1);
    ans = -best[n - 1];
  }

  i64 answer() const { return ans; }
};
