#pragma once
// ライブラリを使う提出が共有するもの。比べたいのは最大流の解き方なので、
// 解法そのものはここに 1 つ置いて、提出はエンジンを選ぶだけにする。
#include "pj.hpp"
#include "mylib/optimization/MaxFlow.hpp"

template <class Engine> struct FlowSolver {
  using MF = MaxFlow<Engine>;

  int n;
  vector<array<int, 2>> edges;
  vector<array<int, 3>> qs;
  vector<i64> ans;

  FlowSolver(int n, const vector<array<int, 2>> &edges,
             const vector<array<int, 3>> &qs)
      : n(n), edges(edges), qs(qs) {}

  void run() {
    MF g(n);
    vector<typename MF::EdgePtr> es;
    // クエリで触られる辺も、容量 0 で先に張っておく。あとから足せないため。
    vector<vector<int>> idx(n, vector<int>(n, -1));
    for (auto &e : edges) {
      idx[e[0]][e[1]] = (int)es.size();
      es.emplace_back(g.add_edge(e[0], e[1], 1, true));
    }
    for (auto &e : qs)
      if (idx[e[1]][e[2]] == -1) {
        idx[e[1]][e[2]] = (int)es.size();
        es.emplace_back(g.add_edge(e[1], e[2], 0, true));
      }

    i64 flow = g.maxflow(0, n - 1);
    ans.clear();
    ans.reserve(qs.size());
    for (auto &e : qs) {
      auto &edge = es[idx[e[1]][e[2]]];
      // 容量を変えると、その辺に乗っていた流れがほどける。ほどけたぶんを
      // 引いてから、1 だけ流し直す。
      flow -= edge.change_cap(e[0] == 1, 0, n - 1);
      flow += g.maxflow(0, n - 1, 1);
      ans.push_back(flow);
    }
  }

  const vector<i64> &answer() const { return ans; }
};
