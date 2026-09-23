#pragma once
#include "common.hpp"
#include "mylib/graph/BipartiteGraph.hpp"

// 二部グラフ専用の最大マッチング。前回の組を引き継いで、崩れたところだけ
// 増加路を探し直す。
struct BipartitePolicy {
  using G = BipartiteGraph;
  static G make(int n) { return G(n, n); }
  static auto matching(const G &g, const vector<int> &partner) {
    return bipartite_matching(g, partner);
  }
};

using Solver = MatchSolver<BipartitePolicy>;
