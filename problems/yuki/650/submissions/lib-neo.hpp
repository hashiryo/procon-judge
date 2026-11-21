#pragma once
#include "pj.hpp"
#include "neo/base/core/algebra/ZMod.hpp"
#include "neo/base/ds/tree/PathProduct.hpp"

// NeoLibrary の HeavyLightDecomposition と PathProduct (鎖ごとの二分木、1 点の代入と道の積が O(log N)) で解く。辺の行列は子の側の
// 頂点に置く。u は v の先祖なので、u から v への道の辺は、u の v の側の子 c から v までの頂点で、PathProduct の c から v への積が
// 根の側を左にした積になる。
struct Solver {
  using Z = ZMod<1000000007>;
  struct MatProd {  // 2x2 行列 (x0 x1; x2 x3) の積
    using T = array<Z, 4>;
    static T ti() { return {Z(1), Z(0), Z(0), Z(1)}; }
    static T op(const T &a, const T &b) {
      return {a[0] * b[0] + a[1] * b[2], a[0] * b[1] + a[1] * b[3], a[2] * b[0] + a[3] * b[2], a[2] * b[1] + a[3] * b[3]};
    }
  };
  HeavyLightDecomposition h;
  vector<int> child;  // 辺 -> 子の側の頂点
  PathProduct<MatProd> pp;
  static vector<int> children(const HeavyLightDecomposition &h, const vector<array<int, 2>> &edges) {
    vector<int> c(edges.size());
    for (size_t i = 0; i < edges.size(); ++i) c[i] = h.parent(edges[i][0]) == edges[i][1] ? edges[i][0] : edges[i][1];
    return c;
  }
  Solver(int n, const vector<array<int, 2>> &edges) : h(n, edges), child(children(h, edges)), pp(h) {}
  void set(int e, const array<i64, 4> &x) { pp.set(child[e], {Z(x[0]), Z(x[1]), Z(x[2]), Z(x[3])}); }
  array<i64, 4> product(int u, int v) {
    if (u == v) return {1, 0, 0, 1};
    const auto m = pp.prod(h.jump(u, v, 1), v);
    return {(i64)m[0].val(), (i64)m[1].val(), (i64)m[2].val(), (i64)m[3].val()};
  }
};
