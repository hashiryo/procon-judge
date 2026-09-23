#pragma once
#include "pj.hpp"
#include "mylib/data_structure/LinkCutTree.hpp"

// Link-Cut 木。木の形を変えられる構造なので、固定の木に対しては過剰だが、
// LCA は expose 2 回で出る。前処理は N 回の link で、償却 O(N log N)。
struct Solver {
  LinkCutTree<> lct;

  Solver(int n, const vector<int> &par) : lct(n) {
    for (int i = 1; i < n; ++i) lct.link(i, par[i]);
    lct.evert(0);
  }

  int lca(int u, int v) { return lct.lca(u, v); }
};
