#pragma once
#include "pj.hpp"
#include "neo/base/ds/tree/TopTree.hpp"

// NeoLibrary の TopTree を使う提出。値は 64 bit、作用は加算 (長さを掛ける)。
struct Solver {
  struct AddSum {
    using T = i64;
    using E = i64;
    using commute = void;
    static T ti() { return 0; }
    static T op(T a, T b) { return a + b; }
    static void mp(T &v, E e, int len) { v += e * (i64)len; }
    static void cp(E &pre, E suf) { pre += suf; }
  };
  TopTree<AddSum> tt;
  Solver(int n, const vector<i64> &a, const vector<pair<int, int>> &edges) : tt(a) {
    for (auto [u, v] : edges) tt.link(u, v);
  }
  void cut(int u, int v) { tt.cut(u, v); }
  void link(int u, int v) { tt.link(u, v); }
  void subtree_add(int v, int p, i64 x) { tt.apply_subtree(v, p, x); }
  i64 subtree_sum(int v, int p) { return tt.prod_subtree(v, p); }
};
