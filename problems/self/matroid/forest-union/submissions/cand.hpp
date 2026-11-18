#pragma once
// NeoLibrary に置く候補の口 mu::matroid_union (problems/_shared/matroid/union_algos.hpp) を使う。中身は union_bfs の
// Frozen、Add、Greedy (Fresh なし) で、型の操作は UnionAdapter を通して呼ぶ。
#include "_shared/matroid/union_algos.hpp"

struct Solver {
  int n, k;
  const vector<array<int, 2>> &edges;
  vector<int> ans;

  Solver(int n, const vector<array<int, 2>> &edges, int k) : n(n), k(k), edges(edges) {}

  void run() {
    vector<mu::Graphic> ms(k, mu::Graphic(n, edges));
    ans = mu::matroid_union(ms);
  }

  vector<int> answer() const { return ans; }
};
