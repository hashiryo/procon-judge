#pragma once
// ライブラリを使う提出が共有するもの。最小全域木と木の上の距離、木に無い辺を
// 点にするところ、質問ごとの場合分けはどちらの実装でも同じなので、ここに置く。
// 比べたいのは静的な 2 次元の点集合に対する長方形の最小を取る構造。
#include <algorithm>
#include <utility>
#include "pj.hpp"
#include "mylib/algebra/ModInt.hpp"
#include "mylib/data_structure/UnionFind.hpp"
#include "mylib/graph/Graph.hpp"
#include "mylib/graph/HeavyLightDecomposition.hpp"

using Mint = ModInt<1000000007>;

struct RangeMin {
  using T = int;
  static T ti() { return 0x7fffffff; }
  static T op(T a, T b) { return std::min(a, b); }
};

// 辺 i の長さが 2^(i+1) なので、番号順の Kruskal がそのまま最小全域木。木の上の
// 距離は根からの距離と LCA で出す。木に無い辺は、両端を列の位置に直した点として
// 持ち、値は辺の番号。
struct Forest {
  int n, m;
  vector<array<int, 2>> es;
  vector<char> used;
  Graph g;
  HeavyLightDecomposition tree;
  vector<Mint> dep;
  vector<array<int, 3>> xyw;  // {列の位置の小さい方, 大きい方, 辺の番号}

  Forest(int n, const vector<array<int, 2>> &edges)
      : n(n), m((int)edges.size()), es(edges), used(m, 0), g(n) {
    vector<Mint> c;
    Mint w = 1;
    UnionFind uf(n);
    for (int i = 0; i < m; ++i) {
      w += w;
      if (uf.unite(es[i][0], es[i][1])) used[i] = 1, g.add_edge(es[i][0], es[i][1]), c.push_back(w);
    }
    tree = HeavyLightDecomposition(g);
    auto adj = g.adjacency_edge(0);
    dep.assign(n, Mint());
    for (int i = 0; i < n; ++i) {
      int v = tree.to_vertex(i);
      for (int e : adj[v])
        if (int u = g[e].to(v); u != tree.parent(v)) dep[u] = dep[v] + c[e];
    }
    for (int i = 0; i < m; ++i) {
      if (used[i]) continue;
      int a = tree.to_seq(es[i][0]), b = tree.to_seq(es[i][1]);
      if (a > b) std::swap(a, b);
      xyw.push_back({a, b, i});
    }
  }

  Mint dist(int u, int v) const { return dep[u] + dep[v] - dep[tree.lca(u, v)] * 2; }
};

// Index は次を実装する。
//   Index(const vector<array<int, 3>> &xyw);      // 点 (x, y) と値
//   int min_in(int x0, int x1, int y0, int y1);   // [x0, x1) x [y0, y1) の最小。無ければ RangeMin::ti()
template <class Index> struct DetourSolver {
  int n;
  vector<array<int, 2>> edges;
  vector<array<int, 3>> qs;
  vector<i64> ans;

  DetourSolver(int n, const vector<array<int, 2>> &edges, const vector<array<int, 3>> &qs)
      : n(n), edges(edges), qs(qs) {}

  void run() {
    Forest f(n, edges);
    Index idx(f.xyw);
    ans.clear();
    for (auto &q : qs) {
      int u = q[0], v = q[1], e = q[2];
      auto [x, y] = f.es[e];
      if (f.tree.parent(y) == x) std::swap(x, y);  // x を子側にする
      const bool u_in = f.tree.in_subtree(u, x);
      // 木に無い辺を避けるか、u と v が同じ側なら、木の上の道がそのまま答え。
      if (!f.used[e] || u_in == f.tree.in_subtree(v, x)) {
        ans.push_back(f.dist(u, v).val());
        continue;
      }
      // 部分木 x とその外をまたぐ、木に無い辺のうち番号が最小のもの。
      auto [l, r] = f.tree.subtree(x);
      int i = std::min(idx.min_in(0, l, l, r), idx.min_in(l, r, r, n));
      if (i > f.m) {
        ans.push_back(-1);
        continue;
      }
      auto [p, w] = f.es[i];
      if (!u_in) std::swap(u, v);
      if (f.tree.in_subtree(w, x)) std::swap(p, w);
      ans.push_back((f.dist(u, p) + f.dist(v, w) + Mint(2).pow(i + 1)).val());
    }
  }

  const vector<i64> &answer() const { return ans; }
};
