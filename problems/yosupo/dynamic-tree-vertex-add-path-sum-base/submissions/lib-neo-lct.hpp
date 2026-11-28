#pragma once
#include "pj.hpp"
#include "neo/base/ds/tree/LinkCutTree.hpp"

// Link-Cut Tree。道の和は可換なので、逆向きの和は持たない。
struct Solver {
  struct SumM {
    using T = i64;
    using commute = void;
    static T ti() { return 0; }
    static T op(T a, T b) { return a + b; }
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
  LinkCutTree<SumM> lct;
  Solver(int n, const vector<i64> &a, const vector<pair<int, int>> &edges): lct(n) {
    for (int i = 0; i < n; ++i) lct.init_value(i, a[i]);
    const vector<int> par = parents(n, edges);
    for (int v = 1; v < n; ++v) lct.init_parent(v, par[v]);
  }
  void cut(int u, int v) { lct.cut(u, v); }
  void link(int u, int v) { lct.link(u, v); }
  void add(int v, i64 x) { lct.mul(v, x); }
  i64 prod(int u, int v) { return lct.prod(u, v); }
};
