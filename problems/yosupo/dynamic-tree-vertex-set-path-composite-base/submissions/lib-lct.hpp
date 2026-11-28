#pragma once
#include "pj.hpp"
#include "mylib/data_structure/LinkCutTree.hpp"
#include "mylib/algebra/ModInt.hpp"

// 今の Library の Link-Cut Tree。比較の基準。
struct Solver {
  using Mint = ModInt<998244353>;
  struct RcompositeQ {
    using T = pair<Mint, Mint>;
    static T op(const T &l, const T &r) { return {r.first * l.first, r.first * l.second + r.second}; }
  };
  LinkCutTree<RcompositeQ> lct;
  Solver(int n, const vector<pair<u32, u32>> &a, const vector<pair<int, int>> &edges): lct(n) {
    for (int i = 0; i < n; ++i) lct.set(i, {Mint(a[i].first), Mint(a[i].second)});
    for (auto [u, v] : edges) lct.link(u, v);
  }
  void cut(int u, int v) { lct.cut(u, v); }
  void link(int u, int v) { lct.link(u, v); }
  void set(int v, u32 c, u32 d) { lct.set(v, {Mint(c), Mint(d)}); }
  u32 prod(int u, int v, u32 x) {
    const auto [c, d] = lct.prod(u, v);
    return u32((c * Mint(x) + d).val());
  }
};
