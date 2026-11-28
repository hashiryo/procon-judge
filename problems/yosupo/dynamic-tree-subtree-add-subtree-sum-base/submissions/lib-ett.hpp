#pragma once
#include "pj.hpp"
#include "mylib/data_structure/EulerTourTree.hpp"

// 今の Library の Euler Tour Tree。比較の基準。
struct Solver {
  struct AddM {
    using T = i64;
    using E = i64;
    static T ti() { return 0; }
    static T op(T a, T b) { return a + b; }
    static void mp(T &v, E e, size_t len) { v += e * (i64)len; }
    static void cp(E &pre, E suf) { pre += suf; }
  };
  EulerTourTree<AddM> ett;
  Solver(int n, const vector<i64> &a, const vector<pair<int, int>> &edges): ett(n) {
    for (int i = 0; i < n; ++i) ett.set(i, a[i]);
    for (auto [u, v] : edges) ett.link(v, u);
  }
  void cut(int u, int v) { ett.cut(u, v); }
  void link(int u, int v) { ett.link(u, v); }
  void subtree_add(int v, int p, i64 x) { ett.apply_subtree(v, p, x); }
  i64 subtree_sum(int v, int p) { return ett.prod_subtree(v, p); }
};
