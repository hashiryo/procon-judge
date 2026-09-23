#pragma once
#include "pj.hpp"
#include "mylib/graph/CliqueProblem.hpp"

// 一般グラフ向けの最小頂点被覆。補グラフの最大クリークに落として枝刈り探索
// する。二部であることを使わないぶん、頂点数が増えると重くなる。
struct Solver {
  int n;
  vector<array<int, 2>> edges;
  i64 ans = 0;

  Solver(int n, const vector<array<int, 2>> &edges) : n(n), edges(edges) {}

  void run() {
    CliqueProblem g(n);
    for (auto &e : edges) g.add_edge(e[0], e[1]);
    ans = (i64)g.get_min_vertex_cover().size();
  }

  i64 answer() const { return ans; }
};
