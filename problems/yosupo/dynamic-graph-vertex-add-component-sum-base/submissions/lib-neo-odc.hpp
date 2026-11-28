#pragma once
#include "pj.hpp"
#include "neo/base/ds/tree/OnlineDynamicConnectivity.hpp"

// オンラインの動的連結性 (HLT の段つき ETT)。置き場は型ごとに 1 つなので、
// 頂点数と操作数から NODE_SIZE を決めて渡す。
struct Solver {
  struct SumM {
    using T = i64;
    using commute = void;
    static T ti() { return 0; }
    static T op(T a, T b) { return a + b; }
  };
  OnlineDynamicConnectivity<SumM, 6'000'000> odc;
  Solver(int n, const vector<i64> &a): odc(n) {
    for (int i = 0; i < n; ++i) odc.set(i, a[i]);
  }
  void link(int u, int v) { odc.link(u, v); }
  void cut(int u, int v) { odc.cut(u, v); }
  void add(int v, i64 x) { odc.set(v, odc[v] + x); }
  i64 component_sum(int v) { return odc.prod(v); }
};
