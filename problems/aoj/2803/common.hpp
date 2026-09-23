#pragma once
// ライブラリを使う提出が共有するもの。比べたいのは最大流の解き方なので、
// 解法そのものはここに 1 つ置いて、提出はエンジンを選ぶだけにする。
#include <algorithm>
#include "pj.hpp"
#include "mylib/optimization/MaxFlow.hpp"

template <class Engine> struct FlowSolver {
  using MF = MaxFlow<Engine>;

  // 答えの上限。これ以上流れたら湯が溢れる。
  static constexpr i64 INF = 512345;

  int k, n;
  vector<array<i64, 3>> edges;
  i64 ans = 0;

  FlowSolver(int k, int n, const vector<array<i64, 3>> &edges)
      : k(k), n(n), edges(edges) {}

  void run() {
    MF g(n + k + 1);
    int src = g.add_vertex();
    for (int j = 1; j <= k; ++j) g.add_edge(src, j, INF);
    vector<typename MF::EdgePtr> es;
    es.reserve(edges.size());
    for (auto &e : edges) es.emplace_back(g.add_edge((int)e[0], (int)e[1], e[2], true));

    ans = g.maxflow(src, 0);
    auto s = g.mincut(src);
    i64 add = 0;
    // カットに乗っている辺を 1 本だけ広げたときの増分が最大になるものを探す。
    for (auto &e : es)
      if (s[e.src()] != s[e.dst()]) {
        i64 pre = e.cap();
        e.change_cap(INF, src, 0);
        add = std::max(add, g.maxflow(src, 0));
        e.change_cap(pre, src, 0);
      }
    ans += add;
  }

  i64 answer() const { return ans; }

  bool unbounded() const { return ans >= INF; }
};
