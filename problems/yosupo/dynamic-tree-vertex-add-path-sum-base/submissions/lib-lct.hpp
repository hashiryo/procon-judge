#pragma once
#include "pj.hpp"
#include "mylib/data_structure/LinkCutTree.hpp"

// 今の Library の Link-Cut Tree。比較の基準。
struct Solver {
  struct RsumQ {
    using T = i64;
    static T op(const T &l, const T &r) { return l + r; }
    using commute = void;
  };
  LinkCutTree<RsumQ> lct;
  Solver(int n, const vector<i64> &a, const vector<pair<int, int>> &edges): lct(n) {
    for (int i = 0; i < n; ++i) lct.set(i, a[i]);
    for (auto [u, v] : edges) lct.link(v, u);
  }
  void cut(int u, int v) { lct.cut(u, v); }
  void link(int u, int v) { lct.link(u, v); }
  void add(int v, i64 x) { lct.set(v, lct[v] + x); }
  i64 prod(int u, int v) { return lct.prod(u, v); }
};
