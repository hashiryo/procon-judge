#pragma once
#include "pj.hpp"
#include "neo/algebra/ZMod.hpp"
#include "neo/data_structure/segtree/AssignProd.hpp"
#include "neo/data_structure/segtree/Composite.hpp"

// NeoLibrary の SegmentTree に monoid::AssignProd<monoid::Composite<ZMod<998244353>>> を渡す。中身は、代入した値の
// 2 の冪乗の表を持つ二分木の遅延セグ木 (lazy_pow と同じ形)。
struct Solver {
  using Z = ZMod<998244353>;
  using CM = monoid::Composite<Z>;
  SegmentTree<monoid::AssignProd<CM>> seg;
  explicit Solver(const vector<array<i64, 2>> &f) : seg(int(f.size()), [&](int i) { return CM::T{Z(f[i][0]), Z(f[i][1])}; }) {}
  void assign(int l, int r, i64 c, i64 d) { seg.apply(l, r, {Z(c), Z(d)}); }
  i64 query(int l, int r, i64 x) { return CM::eval(seg.prod(l, r), Z(x)).val(); }
};
