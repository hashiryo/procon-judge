#pragma once
#include "_sum_exp.hpp"
#include "neo/graph/HeavyLightDecomposition.hpp"

// lib-neo の切り分け。NeoLibrary の HeavyLightDecomposition はそのままで、区間和の木を、作る段で一時の配列を作らない写し
// (_sum_exp.hpp の SumExp<false>) に替え、位置ごとの値の vector も作らずに init で渡す。prod は NeoLibrary と同じ。
struct Solver {
  HeavyLightDecomposition h;
  SumExp<false> seg;
  Solver(int n, const vector<i64> &a, const vector<int> &par) : h(par), seg(n, [&](int i) { return a[h.to_vertex(i)]; }) {}
  void add(int u, i64 x) { seg.mul(h.to_seq(u), x); }
  i64 subtree_sum(int u) {
    auto [l, r] = h.subtree(u);
    return seg.prod(l, r);
  }
};
