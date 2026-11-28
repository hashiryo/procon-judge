#pragma once
#include "pj.hpp"
#include "_shared/dyntree/top_tree.hpp"

// 自己調整 top tree (nandhagk の 337329 の形)。値は 64 bit、作用は加算。
struct Solver {
  LazyTopTree tt;
  Solver(int n, const vector<i64> &a, const vector<pair<int, int>> &edges) : tt(a) {
    for (auto [u, v] : edges) tt.link(u, v);
  }
  void cut(int u, int v) { tt.cut(u, v); }
  void link(int u, int v) { tt.link(u, v); }
  void subtree_add(int v, int p, i64 x) { tt.apply_subtree(v, p, x); }
  i64 subtree_sum(int v, int p) { return tt.prod_subtree(v, p); }
};
