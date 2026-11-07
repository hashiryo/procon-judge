#pragma once
#include "pj.hpp"
#include "neo/algebra/ZMod.hpp"
#include "neo/data_structure/SegmentTree.hpp"

// NeoLibrary の SegmentTree に自前の 1 次関数の合成を渡す。標準の構造体ではないので、中身は二分木の汎用の形。
struct Solver {
  using Z = ZMod<998244353>;
  struct Composite {
    using T = array<Z, 2>;  // x -> t[0] x + t[1]
    static T ti() { return {Z(1), Z(0)}; }
    static T op(const T &l, const T &r) { return {r[0] * l[0], r[0] * l[1] + r[1]}; }
  };
  SegmentTree<Composite> seg;
  explicit Solver(const vector<array<i64, 2>> &f) : seg(int(f.size()), [&](int i) { return Composite::T{Z(f[i][0]), Z(f[i][1])}; }) {}
  void set(int p, i64 c, i64 d) { seg.set(p, {Z(c), Z(d)}); }
  i64 query(int l, int r, i64 x) {
    const auto t = seg.prod(l, r);
    return (t[0] * Z(x) + t[1]).val();
  }
};
