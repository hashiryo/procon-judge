#pragma once
#include "pj.hpp"
#include "neo/algebra/ZMod.hpp"
#include "neo/data_structure/segtree/Composite.hpp"

// NeoLibrary の SegmentTree に、1 次関数の合成のモノイド monoid::Composite<ZMod<998244353>> を渡す。中身は二分木の汎用の形。
struct Solver {
  using Z = ZMod<998244353>;
  using CM = monoid::Composite<Z>;
  SegmentTree<CM> seg;
  explicit Solver(const vector<array<i64, 2>> &f) : seg(int(f.size()), [&](int i) { return CM::T{Z(f[i][0]), Z(f[i][1])}; }) {}
  void set(int p, i64 c, i64 d) { seg.set(p, {Z(c), Z(d)}); }
  i64 query(int l, int r, i64 x) { return CM::eval(seg.prod(l, r), Z(x)).val(); }
};
