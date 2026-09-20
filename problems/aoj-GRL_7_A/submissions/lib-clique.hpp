#pragma once
#include "pj.hpp"
#include "mylib/graph/CliqueProblem.hpp"

// 最小頂点被覆を直接求める。二部であることを使わない一般グラフ向けの実装で、
// 補グラフの最大クリークに落として枝刈り探索する。König の定理により、
// 二部グラフでは最小頂点被覆の大きさが最大マッチングの本数に一致する。
struct Solver {
  int x, y;
  vector<array<int, 2>> edges;
  i64 ans = 0;

  Solver(int x, int y, const vector<array<int, 2>> &edges)
      : x(x), y(y), edges(edges) {}

  void run() {
    CliqueProblem g(x + y);
    for (auto &e : edges) g.add_edge(e[0], x + e[1]);
    ans = (i64)g.get_min_vertex_cover().size();
  }

  i64 answer() const { return ans; }
};
