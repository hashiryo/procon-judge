#pragma once
// problems/_shared/matroid/union_algos.hpp の union_bfs (Edmonds のマトロイドの分割) で、Frozen と Fresh に加えて、そのまま入る辺を build し直さずに足す (Add)。
#include "_shared/matroid/union_algos.hpp"

struct Solver {
  int n, k;
  const vector<array<int, 2>> &edges;
  vector<int> ans;

  Solver(int n, const vector<array<int, 2>> &edges, int k) : n(n), k(k), edges(edges) {}

  void run() {
    vector<mu::Graphic> ms(k, mu::Graphic(n, edges));
    ans = mu::union_bfs<true, true, true, false>(ms);
  }

  vector<int> answer() const { return ans; }
};
