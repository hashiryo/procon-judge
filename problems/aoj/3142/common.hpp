#pragma once
// ライブラリを使う提出が共有するもの。比べたいのは重み付き Union-Find の
// 持ち方なので、解法はここに 1 つ置いて、提出は型を選ぶだけにする。
#include <algorithm>
#include "pj.hpp"
#include "mylib/graph/Graph.hpp"
#include "mylib/graph/incidence_matrix_equation.hpp"

template <class UF> struct PotentialSolver {
  int n;
  vector<array<int, 2>> edges;
  vector<i64> d;
  i64 ans = 0;

  PotentialSolver(int n, const vector<array<int, 2>> &edges, const vector<i64> &d)
      : n(n), edges(edges), d(d) {}

  void run() {
    Graph g(n);
    for (auto &e : edges) g.add_edge(e[0], e[1]);
    auto sol = incidence_matrix_equation(g, d);
    UF uf(n);
    for (int i = 0; i < n - 1; ++i) {
      auto [u, v] = g[i];
      uf.unite(v, u, sol[i]);
    }
    // ポテンシャルは定数のずれを持つので、いちばん低いところを 0 に寄せる。
    i64 sum = 0, lo = 0;
    for (int i = 0; i < n; ++i) {
      i64 p = uf.potential(i);
      sum += p, lo = std::min(lo, p);
    }
    ans = sum - lo * n;
  }

  i64 answer() const { return ans; }
};
