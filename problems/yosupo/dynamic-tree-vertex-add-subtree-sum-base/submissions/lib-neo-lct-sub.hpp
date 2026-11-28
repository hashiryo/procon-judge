#pragma once
#include "pj.hpp"
#include "neo/base/ds/tree/LinkCutTree.hpp"

// 部分木の和を答える Link-Cut Tree。可換群 (逆元あり) を受ける。
struct Solver {
  struct SumG {
    using T = i64;
    using commute = void;
    static T ti() { return 0; }
    static T op(T a, T b) { return a + b; }
    static T inv(T a) { return -a; }
  };
  static vector<int> parents(int n, const vector<pair<int, int>> &edges) {
    vector<vector<int>> g(n);
    for (auto [u, v] : edges) g[u].push_back(v), g[v].push_back(u);
    vector<int> par(n, -1), st{0};
    par[0] = 0;
    while (!st.empty()) {
      int x = st.back();
      st.pop_back();
      for (int y : g[x])
        if (par[y] < 0) par[y] = x, st.push_back(y);
    }
    return par;
  }
  SubtreeLinkCutTree<SumG> lct;
  Solver(int n, const vector<i64> &a, const vector<pair<int, int>> &edges): lct(n) {
    for (int i = 0; i < n; ++i) lct.set(i, a[i]);
    const vector<int> par = parents(n, edges);
    for (int v = 1; v < n; ++v) lct.link(v, par[v]);
  }
  void cut(int u, int v) { lct.cut(u, v); }
  void link(int u, int v) { lct.link(u, v); }
  void add(int v, i64 x) { lct.add(v, x); }
  i64 subtree_sum(int v, int p) { return lct.prod_subtree(v, p); }
};
