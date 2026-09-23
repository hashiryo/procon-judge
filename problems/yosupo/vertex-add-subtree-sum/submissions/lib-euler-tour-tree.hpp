#pragma once
#include "pj.hpp"
#include "mylib/data_structure/EulerTourTree.hpp"

// Euler Tour 木。木の形を変えられる構造なので固定の木には過剰だが、部分木の
// 総和を直接持てる。構築は N 回の link で、ノードは静的な配列から取る。
struct Solver {
  struct RangeSum {
    using T = i64;
    static T ti() { return 0; }
    static T op(T a, T b) { return a + b; }
  };

  vector<int> par;
  EulerTourTree<RangeSum> ett;

  Solver(int n, const vector<i64> &a, const vector<int> &par)
      : par(par), ett(n) {
    for (int u = 0; u < n; ++u) ett.set(u, a[u]);
    for (int i = 1; i < n; ++i) ett.link(par[i], i);
  }

  void add(int u, i64 x) { ett.set(u, ett.get(u) + x); }

  i64 subtree_sum(int u) { return ett.prod_subtree(u, par[u]); }
};
