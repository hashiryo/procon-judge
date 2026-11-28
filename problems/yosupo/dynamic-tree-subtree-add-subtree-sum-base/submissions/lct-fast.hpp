#pragma once
#include "pj.hpp"
#include "_shared/dyntree/lct_fast.hpp"

// chaihf の 278245 の形の、部分木加算付き Link-Cut Tree。
struct Solver {
  LctFast lct;
  Solver(int n, const vector<i64> &a, const vector<pair<int, int>> &edges) : lct(a, edges) {}
  void cut(int u, int v) { lct.cut(u, v); }
  void link(int u, int v) { lct.link(u, v); }
  void subtree_add(int v, int p, i64 x) { lct.apply_subtree(v, p, x); }
  i64 subtree_sum(int v, int p) { return lct.prod_subtree(v, p); }
};
