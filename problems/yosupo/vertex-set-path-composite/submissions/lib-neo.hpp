#pragma once
#include "pj.hpp"
#include "neo/base/core/algebra/ZMod.hpp"
#include "neo/base/ds/range/segtree/Composite.hpp"
#include "neo/base/ds/tree/PathProduct.hpp"

// NeoLibrary の HeavyLightDecomposition を辺の列から作り、PathProduct<monoid::Composite<ZMod<998244353>>> (鎖ごとの二分木、
// 1 回の更新と問い合わせが O(log N)) で道の合成を求める。
struct Solver {
  using Z = ZMod<998244353>;
  using M = monoid::Composite<Z>;
  PathProduct<M> pp;
  static vector<M::T> init(const vector<array<i64, 2>> &f) {
    vector<M::T> v(f.size());
    for (size_t i = 0; i < f.size(); ++i) v[i] = {Z(f[i][0]), Z(f[i][1])};
    return v;
  }
  Solver(int n, const vector<array<i64, 2>> &f, const vector<array<int, 2>> &edges)
      : pp(HeavyLightDecomposition(n, edges), init(f)) {}
  void set(int p, i64 c, i64 d) { pp.set(p, {Z(c), Z(d)}); }
  i64 composite(int u, int v, i64 x) { return M::eval(pp.prod(u, v), Z(x)).val(); }
};
