#pragma once
// problems/_shared/matroid/union_algos.hpp の union_bfs (Edmonds のマトロイドの分割) で、Frozen に加えて、探索 1 回の中で一度報告した辺を二度報告しない閉路の列挙 (Fresh) を使う。
#include "_shared/matroid/union_algos.hpp"

struct Solver {
  int n, k;
  const vector<array<int, 2>> &edges;
  vector<int> ans;

  Solver(int n, const vector<array<int, 2>> &edges, int k) : n(n), k(k), edges(edges) {}

  void run() {
    vector<mu::Graphic> ms(k, mu::Graphic(n, edges));
    ans = mu::union_bfs<true, true, false, false>(ms);
  }

  vector<int> answer() const { return ans; }
};
