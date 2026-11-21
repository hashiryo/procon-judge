#pragma once
#include "_sum_exp.hpp"
#include "neo/base/ds/tree/HeavyLightDecomposition.hpp"

// lib-neo-lean の prod を、l と r が同じ子に入った段で止める形 (SumExp<true>) にしたもの。部分木は小さいものが多く、
// 上の段を読まずに済む。
struct Solver {
  HeavyLightDecomposition h;
  SumExp<true> seg;
  Solver(int n, const vector<i64> &a, const vector<int> &par) : h(par), seg(n, [&](int i) { return a[h.to_vertex(i)]; }) {}
  void add(int u, i64 x) { seg.mul(h.to_seq(u), x); }
  i64 subtree_sum(int u) {
    auto [l, r] = h.subtree(u);
    return seg.prod(l, r);
  }
};
