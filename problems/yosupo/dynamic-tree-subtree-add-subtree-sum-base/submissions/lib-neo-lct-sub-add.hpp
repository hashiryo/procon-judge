#pragma once
#include "pj.hpp"
#include "neo/base/ds/tree/SubtreeAddLinkCutTree.hpp"

// NeoLibrary の SubtreeAddLinkCutTree を使う提出 (416793 の形)。和の群と「足す」作用は、どちらも逆元が -x。
struct Solver {
  struct AddSum {
    using T = i64;
    using E = i64;
    using commute = void;
    static T ti() { return 0; }
    static T op(T a, T b) { return a + b; }
    static T inv(T a) { return -a; }
    static void mp(T &v, E e, int len) { v += e * (i64)len; }
    static void cp(E &pre, E suf) { pre += suf; }
    static E inve(E e) { return -e; }
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

  SubtreeAddLinkCutTree<AddSum> lct;
  Solver(int n, const vector<i64> &a, const vector<pair<int, int>> &edges) : lct(n) {
    for (int i = 0; i < n; ++i) lct.set(i, a[i]);
    const vector<int> par = parents(n, edges);
    for (int v = 1; v < n; ++v) lct.link(v, par[v]);
  }
  void cut(int u, int v) { lct.cut(u, v); }
  void link(int u, int v) { lct.link(u, v); }
  void subtree_add(int v, int p, i64 x) { lct.apply_subtree(v, p, x); }
  i64 subtree_sum(int v, int p) { return lct.prod_subtree(v, p); }
};
