#pragma once
#include "pj.hpp"
#include "_shared/dyntree/ett_treap.hpp"

// Euler Tour Tree (splay 木)。部分木への作用は遅延で持つ。
struct Solver {
  struct AddM {
    using T = i64;
    using E = i64;
    static T ti() { return 0; }
    static T op(T a, T b) { return a + b; }
    static void mp(T &v, E e, int len) { v += e * (i64)len; }
    static void cp(E &pre, E suf) { pre += suf; }
  };
  static vector<array<int, 2>> to_edges(const vector<pair<int, int>> &e) {
    vector<array<int, 2>> r(e.size());
    for (size_t i = 0; i < e.size(); ++i) r[i] = {e[i].first, e[i].second};
    return r;
  }
  TreapEulerTourTree<AddM> ett;
  Solver(int n, const vector<i64> &a, const vector<pair<int, int>> &edges): ett(n - 1 < 0 ? n : n, to_edges(edges), a) {}
  void cut(int u, int v) { ett.cut(u, v); }
  void link(int u, int v) { ett.link(u, v); }
  void subtree_add(int v, int p, i64 x) { ett.apply_subtree(v, p, x); }
  i64 subtree_sum(int v, int p) { return ett.prod_subtree(v, p); }
};
