#pragma once
#include "pj.hpp"
#include "mylib/optimization/matroid_intersection.hpp"

// 重み付きマトロイド交叉に落とす。辺の集合について「閉路を作らない」条件が
// グラフマトロイド、「各頂点への入辺が高々 1 本」が分割マトロイドで、その
// 共通部分の最小重み基が求める有向木になる。専用の実装より一般的なぶん遅い。
struct Solver {
  int n, root;
  vector<array<i64, 3>> edges;
  i64 ans = 0;

  Solver(int n, int root, const vector<array<i64, 3>> &edges)
      : n(n), root(root), edges(edges) {}

  void run() {
    const int m = (int)edges.size();
    GraphicMatroid gm(n);
    vector<vector<int>> parts(n);
    vector<i64> w;
    w.reserve(m);
    for (int i = 0; i < m; ++i) {
      gm.add_edge((int)edges[i][0], (int)edges[i][1]);
      parts[(int)edges[i][1]].push_back(i);
      w.push_back(edges[i][2]);
    }
    vector<int> cap(n, 1);
    cap[root] = 0;  // 根には入らない
    PartitionMatroid pm(m, parts, cap);
    auto s = weighted_matroid_intersection<MINIMIZE>(m, gm, pm, w);
    ans = 0;
    for (int e : s[n - 1]) ans += w[e];
  }

  i64 answer() const { return ans; }
};
