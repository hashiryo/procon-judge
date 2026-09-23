#pragma once
// ライブラリを使う提出が共有するもの。比べたいのは最大流の解き方なので、
// 解法そのものはここに 1 つ置いて、提出はエンジンを選ぶだけにする。
#include "pj.hpp"
#include "mylib/optimization/MaxFlow.hpp"

template <class Engine> struct FlowSolver {
  using MF = MaxFlow<Engine>;

  int n;
  vector<array<i64, 3>> edges;
  i64 ans = 0;

  FlowSolver(int n, const vector<array<i64, 3>> &edges) : n(n), edges(edges) {}

  void run() {
    MF g(n);
    vector<typename MF::EdgePtr> es;
    es.reserve(edges.size());
    for (auto &e : edges) es.emplace_back(g.add_edge((int)e[0], (int)e[1], e[2], true));
    ans = g.maxflow(0, n - 1);
    g.mincut(0);
    // 容量 1 の辺を 1 本ずつ塞いで、流量が減るものが 1 本でもあれば減らせる。
    for (auto &e : es)
      if (e.cap() == 1) {
        if (e.change_cap(0, 0, n - 1)) {
          --ans;
          break;
        }
        e.change_cap(1, 0, n - 1);
      }
    if (ans > 10000) ans = -1;
  }

  i64 answer() const { return ans; }
};
