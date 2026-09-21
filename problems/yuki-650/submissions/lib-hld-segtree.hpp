#pragma once
#include "common.hpp"
#include "mylib/graph/Graph.hpp"
#include "mylib/graph/HeavyLightDecomposition.hpp"
#include "mylib/data_structure/SegmentTree.hpp"

// HLD で木を列に潰すと、道が O(log N) 本の区間になる。辺の値は子側の頂点に
// 持たせる。先祖から子孫への道しか聞かれないので、区間は根側から順に並び、
// 反転した積は要らない。
struct Solver {
  int n;
  HeavyLightDecomposition hld;
  vector<int> child;  // 辺 -> 子側の頂点
  SegmentTree<MatProd> seg;

  static HeavyLightDecomposition make_hld(int n, const vector<array<int, 2>> &edges) {
    Graph g(n);
    for (auto &e : edges) g.add_edge(e[0], e[1]);
    return HeavyLightDecomposition(g, 0);
  }

  static vector<int> children(const HeavyLightDecomposition &h,
                              const vector<array<int, 2>> &edges) {
    vector<int> c(edges.size());
    for (size_t i = 0; i < edges.size(); ++i)
      c[i] = h.parent(edges[i][0]) == edges[i][1] ? edges[i][0] : edges[i][1];
    return c;
  }

  Solver(int n, const vector<array<int, 2>> &edges)
      : n(n), hld(make_hld(n, edges)), child(children(hld, edges)), seg(n, IDENTITY) {}

  void set(int e, const array<i64, 4> &x) { seg.set(hld.to_seq(child[e]), to_mat(x)); }

  array<i64, 4> product(int u, int v) {
    Mat acc = IDENTITY;
    for (auto [l, r] : hld.path(u, v, true)) acc *= seg.prod(l, r + 1);
    return from_mat(acc);
  }
};
