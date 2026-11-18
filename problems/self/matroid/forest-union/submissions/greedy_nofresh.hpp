#pragma once
// problems/_shared/matroid/union_algos.hpp の union_bfs (Edmonds のマトロイドの分割) で、greedy.hpp から Fresh だけを外したもの (閉路は毎回全部を列挙する)。
#include "_shared/matroid/union_algos.hpp"

struct Solver {
  int n, k;
  const vector<array<int, 2>> &edges;
  vector<int> ans;

  Solver(int n, const vector<array<int, 2>> &edges, int k) : n(n), k(k), edges(edges) {}

  void run() {
    vector<mu::Graphic> ms(k, mu::Graphic(n, edges));
    ans = mu::union_bfs<true, false, true, true>(ms);
  }

  vector<int> answer() const { return ans; }
};
