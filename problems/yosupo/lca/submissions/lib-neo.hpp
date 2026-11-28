#pragma once
#include "pj.hpp"
#include "neo/base/ds/tree/LinkCutTree.hpp"

// NeoLibrary の値を持たない Link-Cut Tree。par[i] < i なので、親をそのまま仮想辺として張る (O(N))。
// 根は 0 のままなので evert は要らない。
struct Solver {
 LinkCutTree<> lct;
 Solver(int n, const vector<int>& par): lct(n) {
  for (int i = 1; i < n; ++i) lct.init_parent(i, par[i]);
 }
 int lca(int u, int v) { return lct.lca(u, v); }
};
