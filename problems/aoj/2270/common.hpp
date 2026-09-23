#pragma once
// ライブラリを使う提出が共有するもの。解法はここに 1 つ置いて、提出は永続な
// 木の型を選ぶだけにする。
#include "pj.hpp"
#include "mylib/graph/Graph.hpp"
#include "mylib/graph/HeavyLightDecomposition.hpp"

struct CountSum {
  using T = int;
  static T ti() { return 0; }
  static T op(T l, T r) { return l + r; }
};

template <class Seg> struct PathSolver {
  int n;
  vector<i64> x;
  vector<array<int, 2>> edges;
  vector<array<int, 3>> qs;
  vector<i64> ans;

  PathSolver(int n, const vector<i64> &x, const vector<array<int, 2>> &edges,
             const vector<array<int, 3>> &qs)
      : n(n), x(x), edges(edges), qs(qs) {}

  void run() {
    Graph g(n + 1);
    g.add_edge(0, 1);  // 仮の根
    for (auto &e : edges) g.add_edge(e[0], e[1]);
    auto adj = g.adjacency_vertex(0);
    HeavyLightDecomposition tree(adj, 0);

    // 根から各頂点までに現れた値の個数を、永続な木として持つ。
    vector<Seg> segs(n + 1);
    auto dfs = [&](auto &&self, int v, int p) -> void {
      segs[v] = segs[p];
      segs[v].set((int)x[v], segs[v][(int)x[v]] + 1);
      for (int u : adj[v])
        if (u != p) self(self, u, v);
    };
    dfs(dfs, 1, 0);

    ans.clear();
    ans.reserve(qs.size());
    for (auto &e : qs) {
      int v = e[0], w = e[1], l = e[2];
      // segs[v] + segs[w] - segs[lca] - segs[parent(lca)] が道の上の個数。
      auto check = [&](int a, int b, int c, int d) { return a + b - c - d >= l; };
      int lca = tree.lca(v, w), lcap = tree.parent(lca);
      ans.push_back(Seg::template find_first<4>(
          0, check, {segs[v], segs[w], segs[lca], segs[lcap]}));
    }
  }

  const vector<i64> &answer() const { return ans; }
};
