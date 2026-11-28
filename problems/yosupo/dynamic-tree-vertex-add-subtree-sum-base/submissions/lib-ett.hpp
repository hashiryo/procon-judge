#pragma once
#include "pj.hpp"
#include "mylib/data_structure/EulerTourTree.hpp"

// 今の Library の Euler Tour Tree。比較の基準。
struct Solver {
  struct RsumQ {
    using T = i64;
    static T ti() { return 0; }
    static T op(T l, T r) { return l + r; }
  };
  EulerTourTree<RsumQ> ett;
  Solver(int n, const vector<i64> &a, const vector<pair<int, int>> &edges): ett(n) {
    for (int i = 0; i < n; ++i) ett.set(i, a[i]);
    for (auto [u, v] : edges) ett.link(v, u);
  }
  void cut(int u, int v) { ett.cut(u, v); }
  void link(int u, int v) { ett.link(u, v); }
  void add(int v, i64 x) { ett.set(v, ett[v] + x); }
  i64 subtree_sum(int v, int p) { return ett.prod_subtree(v, p); }
};
