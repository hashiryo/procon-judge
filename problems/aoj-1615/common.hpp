#pragma once
// ライブラリを使う提出が共有するもの。比べたいのは最大流のエンジンなので、
// 解法はここに 1 つ置いて、提出はエンジンを選ぶだけにする。
#include "pj.hpp"
#include "mylib/optimization/MaxFlow.hpp"

template <class Engine> struct FlowSolver {
  using MF = MaxFlowLowerBound<Engine>;

  int n;
  vector<array<int, 2>> edges;
  array<i64, 2> ans{0, 0};

  FlowSolver(int n, const vector<array<int, 2>> &edges) : n(n), edges(edges) {}

  void run() {
    const int m = (int)edges.size();
    // 幅 d を 0 から増やし、同じ幅の中では下限 l が大きい方から試す。
    for (int d = 0; d <= n; ++d)
      for (int l = n - d; l >= 0; --l) {
        int r = l + d;
        MF g;
        int s = g.add_vertex(), t = g.add_vertex();
        auto e = g.add_vertices(m);
        auto w = g.add_vertices(n);
        for (int i = 0; i < m; ++i) {
          g.add_edge(s, e[i], 0, 1);
          g.add_edge(e[i], w[edges[i][0]], 0, 1);
          g.add_edge(e[i], w[edges[i][1]], 0, 1);
        }
        // 各頂点が受け持つ本数を [l, r] に収める。
        for (int i = 0; i < n; ++i) g.add_edge(w[i], t, l, r);
        if (g.maxflow(s, t) == m) {
          ans = {l, r};
          return;
        }
      }
  }

  array<i64, 2> answer() const { return ans; }
};
