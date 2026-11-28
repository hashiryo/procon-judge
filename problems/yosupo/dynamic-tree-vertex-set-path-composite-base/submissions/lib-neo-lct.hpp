#pragma once
#include "pj.hpp"
#include "neo/base/ds/tree/LinkCutTree.hpp"

constexpr u32 MOD = 998244353;
// 一次関数 f(x) = a x + b の積。f を先に、g をあとに当てる。
struct CompM {
  using T = pair<u32, u32>;
  static T ti() { return {1, 0}; }
  static T op(const T &f, const T &g) {
    return {u32((u64)g.first * f.first % MOD), u32(((u64)g.first * f.second + g.second) % MOD)};
  }
};
struct Solver {
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
  LinkCutTree<CompM> lct;
  Solver(int n, const vector<pair<u32, u32>> &a, const vector<pair<int, int>> &edges): lct(n) {
    for (int i = 0; i < n; ++i) lct.init_value(i, a[i]);
    const vector<int> par = parents(n, edges);
    for (int v = 1; v < n; ++v) lct.init_parent(v, par[v]);
  }
  void cut(int u, int v) { lct.cut(u, v); }
  void link(int u, int v) { lct.link(u, v); }
  void set(int v, u32 c, u32 d) { lct.set(v, {c, d}); }
  u32 prod(int u, int v, u32 x) {
    const auto f = lct.prod(u, v);
    return u32(((u64)f.first * x + f.second) % MOD);
  }
};
