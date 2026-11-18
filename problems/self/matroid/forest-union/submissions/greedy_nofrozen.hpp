#pragma once
// problems/_shared/matroid/union_algos.hpp の union_bfs (Edmonds のマトロイドの分割) で、greedy.hpp から Frozen だけを外したもの (探索で失敗した要素も以後たどる)。
#include "_shared/matroid/union_algos.hpp"

struct Solver {
  int n, k;
  const vector<array<int, 2>> &edges;
  vector<int> ans;

  Solver(int n, const vector<array<int, 2>> &edges, int k) : n(n), k(k), edges(edges) {}

  void run() {
    vector<mu::Graphic> ms(k, mu::Graphic(n, edges));
    ans = mu::union_bfs<false, true, true, true>(ms);
  }

  vector<int> answer() const { return ans; }
};
