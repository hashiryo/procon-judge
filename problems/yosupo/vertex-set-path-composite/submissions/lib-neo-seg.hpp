#pragma once
#include "pj.hpp"
#include "neo/algebra/ZMod.hpp"
#include "neo/data_structure/segtree/Composite.hpp"
#include "neo/graph/HeavyLightDecomposition.hpp"

// NeoLibrary の HeavyLightDecomposition と、SegmentTree<monoid::Composite<ZMod<998244353>>> 2 本 (位置の順と、その逆順) で
// 道の合成を求める (今の Library と同じ書き方)。上る区間は逆順の木、下りる区間は位置の順の木の積を、道の順に掛ける。
struct Solver {
  using Z = ZMod<998244353>;
  using M = monoid::Composite<Z>;
  int n;
  HeavyLightDecomposition h;
  SegmentTree<M> seg, rseg;
  static vector<M::T> by_pos(const HeavyLightDecomposition &h, const vector<array<i64, 2>> &f, bool rev) {
    const int n = (int)f.size();
    vector<M::T> v(n);
    for (int u = 0; u < n; ++u) v[rev ? n - 1 - h.to_seq(u) : h.to_seq(u)] = {Z(f[u][0]), Z(f[u][1])};
    return v;
  }
  Solver(int n_, const vector<array<i64, 2>> &f, const vector<array<int, 2>> &edges)
      : n(n_), h(n_, edges), seg(by_pos(h, f, false)), rseg(by_pos(h, f, true)) {}
  void set(int p, i64 c, i64 d) {
    const int i = h.to_seq(p);
    seg.set(i, {Z(c), Z(d)}), rseg.set(n - 1 - i, {Z(c), Z(d)});
  }
  i64 composite(int u, int v, i64 x) {
    M::T acc = M::ti();
    h.path(u, v, [&](int l, int r) { acc = M::op(acc, l <= r ? seg.prod(l, r + 1) : rseg.prod(n - 1 - l, n - r)); });
    return M::eval(acc, Z(x)).val();
  }
};
