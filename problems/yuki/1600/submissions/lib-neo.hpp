#pragma once
#include <algorithm>
#include <utility>
#include "pj.hpp"
#include "mylib/data_structure/SegmentTree_2D.hpp"
#include "mylib/data_structure/UnionFind.hpp"
#include "neo/base/core/algebra/ZMod.hpp"
#include "neo/base/ds/tree/HeavyLightDecomposition.hpp"

// common.hpp の DetourSolver と同じ解き方を、木を NeoLibrary の HeavyLightDecomposition に、剰余を ZMod にして書いたもの。最小
// 全域木の Kruskal は今の Library の UnionFind、木に無い辺の点の長方形の最小は lib-segtree-2d と同じ今の Library の 2 次元
// セグメント木 (NeoLibrary に長方形の最小はまだ無い)。common.hpp は今の Library の HLD を読むので、名前がぶつからないよう
// ここでは読まない。
struct Solver {
  using Z = ZMod<1000000007>;
  struct MinMonoid {
    using T = int;
    static T ti() { return 0x7fffffff; }
    static T op(T a, T b) { return std::min(a, b); }
  };
  int n;
  vector<array<int, 2>> edges;
  vector<array<int, 3>> qs;
  vector<i64> ans;
  Solver(int n, const vector<array<int, 2>> &edges, const vector<array<int, 3>> &qs) : n(n), edges(edges), qs(qs) {}
  void run() {
    const int m = (int)edges.size();
    // 辺 i の長さが 2^(i+1) なので、番号順の Kruskal がそのまま最小全域木。
    vector<char> used(m, 0);
    vector<array<int, 2>> te;
    vector<Z> tw;
    Z w = 1;
    UnionFind uf(n);
    for (int i = 0; i < m; ++i) {
      w += w;
      if (uf.unite(edges[i][0], edges[i][1])) used[i] = 1, te.push_back(edges[i]), tw.push_back(w);
    }
    HeavyLightDecomposition tree(n, te);
    // 根からの距離。辺の長さを子の側の頂点に置き、位置の順 (親が子より前) に足す。
    vector<Z> cw(n, Z(0)), dep(n, Z(0));
    for (size_t j = 0; j < te.size(); ++j) cw[tree.parent(te[j][0]) == te[j][1] ? te[j][0] : te[j][1]] = tw[j];
    for (int i = 0; i < n; ++i) {
      const int v = tree.to_vertex(i), p = tree.parent(v);
      if (p >= 0) dep[v] = dep[p] + cw[v];
    }
    auto dist = [&](int u, int v) { return dep[u] + dep[v] - dep[tree.lca(u, v)] * 2; };
    // 木に無い辺は、両端を列の位置に直した点として持ち、値は辺の番号。
    vector<array<int, 3>> xyw;
    for (int i = 0; i < m; ++i) {
      if (used[i]) continue;
      int a = tree.to_seq(edges[i][0]), b = tree.to_seq(edges[i][1]);
      if (a > b) std::swap(a, b);
      xyw.push_back({a, b, i});
    }
    SegmentTree_2D<int, MinMonoid> idx(xyw);
    ans.clear();
    for (auto &q : qs) {
      int u = q[0], v = q[1], e = q[2];
      auto [x, y] = edges[e];
      if (tree.parent(y) == x) std::swap(x, y);  // x を子側にする
      const bool u_in = tree.in_subtree(u, x);
      // 木に無い辺を避けるか、u と v が同じ側なら、木の上の道がそのまま答え。
      if (!used[e] || u_in == tree.in_subtree(v, x)) {
        ans.push_back(dist(u, v).val());
        continue;
      }
      // 部分木 x とその外をまたぐ、木に無い辺のうち番号が最小のもの。
      auto [l, r] = tree.subtree(x);
      const int i = std::min(idx.prod(0, l, l, r), idx.prod(l, r, r, n));
      if (i > m) {
        ans.push_back(-1);
        continue;
      }
      auto [p, wv] = edges[i];
      if (!u_in) std::swap(u, v);
      if (tree.in_subtree(wv, x)) std::swap(p, wv);
      ans.push_back((dist(u, p) + dist(v, wv) + Z(2).pow(i + 1)).val());
    }
  }
  const vector<i64> &answer() const { return ans; }
};
