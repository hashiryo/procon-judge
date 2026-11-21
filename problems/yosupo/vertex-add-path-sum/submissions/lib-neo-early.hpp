#pragma once
#include "_sum_exp.hpp"
#include "neo/base/ds/tree/HeavyLightDecomposition.hpp"

// lib-neo-mm の区間和の木を、作る段で一時の配列を作らず、prod を l と r が同じ子に入った段で止める写し (_sum_exp.hpp の
// SumExp<true>) に替える。乱択の木の道は短い区間に分かれるので、上の段を読まずに済む区間が多い。
struct Solver {
  HeavyLightDecomposition h;
  SumExp<true> seg;
  Solver(int n, const vector<i64> &a, const vector<array<int, 2>> &edges)
      : h(n, edges), seg(n, [&](int i) { return a[h.to_vertex(i)]; }) {}
  void add(int p, i64 x) { seg.mul(h.to_seq(p), x); }
  i64 path_sum(int u, int v) {
    i64 s = 0;
    h.path(u, v, [&](int l, int r) { s += seg.prod(min(l, r), max(l, r) + 1); });
    return s;
  }
};
