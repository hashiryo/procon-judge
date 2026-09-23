#pragma once
#include "common.hpp"
#include "mylib/graph/Graph.hpp"
#include "mylib/graph/HeavyLightDecomposition.hpp"
#include "mylib/data_structure/SegmentTree.hpp"

// HLD で木を列に潰すと、道が O(log N) 本の区間になる。区間の最大はセグメント木。
// 最大を取る成分が分かったら、その成分の値を 1 点更新で置き換える。
struct Solver {
  BridgeTree tree;
  Prey prey;
  HeavyLightDecomposition hld;
  SegmentTree<MaxPrey> seg;

  static HeavyLightDecomposition make_hld(const BridgeTree &t) {
    Graph g(t.n);
    for (auto &e : t.edges) g.add_edge(e[0], e[1]);
    return HeavyLightDecomposition(g, 0);
  }

  static vector<MaxPrey::T> initial(const HeavyLightDecomposition &h, int n) {
    vector<MaxPrey::T> v(n);
    for (int c = 0; c < n; ++c) v[h.to_seq(c)] = {-1, c};
    return v;
  }

  Solver(int n, const vector<array<int, 2>> &edges)
      : tree(contract(n, edges)), prey(tree.n), hld(make_hld(tree)),
        seg(initial(hld, tree.n)) {}

  void add(int u, i64 w) {
    int c = tree.id[u];
    prey.add(c, w);
    seg.set(hld.to_seq(c), {prey.top(c), c});
  }

  i64 take(int s, int t) {
    int u = tree.id[s], v = tree.id[t];
    MaxPrey::T best = MaxPrey::ti();
    for (auto [x, y] : hld.path(u, v))
      best = MaxPrey::op(best, x < y ? seg.prod(x, y + 1) : seg.prod(y, x + 1));
    if (best.first == -1) return -1;
    int c = best.second;
    prey.pop(c);
    seg.set(hld.to_seq(c), {prey.top(c), c});
    return best.first;
  }
};
