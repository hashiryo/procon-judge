#pragma once
// k 本の森の直和と「同じ辺は 1 本まで」の分割マトロイドの交差に直して、NeoLibrary の matroid_intersection (e1f86af) で
// 解く (problems/_shared/matroid/union_algos.hpp の union_isect)。台集合が k 倍になる。
#include "_shared/matroid/union_algos.hpp"

struct Solver {
  int n, k;
  const vector<array<int, 2>> &edges;
  vector<int> ans;

  Solver(int n, const vector<array<int, 2>> &edges, int k) : n(n), k(k), edges(edges) {}

  void run() {
    vector<mu::Graphic> ms(k, mu::Graphic(n, edges));
    ans = mu::union_isect(ms);
  }

  vector<int> answer() const { return ans; }
};
