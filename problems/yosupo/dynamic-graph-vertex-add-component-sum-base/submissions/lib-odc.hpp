#pragma once
#include "pj.hpp"
#include "mylib/data_structure/OnlineDynamicConnectivity.hpp"

// 今の Library の OnlineDynamicConnectivity。比較の基準。
struct Solver {
  struct Monoid {
    using T = i64;
    static T ti() { return 0; }
    static T op(T l, T r) { return l + r; }
  };
  OnlineDynamicConnectivity<Monoid, 6'000'000> odc;
  Solver(int n, const vector<i64> &a): odc(n) {
    for (int i = 0; i < n; ++i) odc.set(i, a[i]);
  }
  void link(int u, int v) { odc.link(u, v); }
  void cut(int u, int v) { odc.cut(u, v); }
  void add(int v, i64 x) { odc.set(v, odc[v] + x); }
  i64 component_sum(int v) { return odc.prod(v); }
};
