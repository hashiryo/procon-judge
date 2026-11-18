#pragma once
#include "pj.hpp"
#include "neo/algebra/ZMod.hpp"
#include "neo/data_structure/Sequence.hpp"

// NeoLibrary の neo/data_structure/Sequence.hpp (塊の形)。monoid::AffineSum<ZMod<P>> を渡すと、塊の大きさが 256 になり、塊の中の
// 和と作用を AVX2 で回す。
struct Solver {
  using Z = ZMod<998244353>;
  Sequence<monoid::AffineSum<Z>, true> t;
  explicit Solver(const vector<i64> &a) : t(vector<Z>(a.begin(), a.end())) {}
  void insert(int i, i64 x) { t.insert(i, Z(x)); }
  void erase(int i) { t.erase(i); }
  void reverse(int l, int r) { t.reverse(l, r); }
  void affine(int l, int r, i64 b, i64 c) { t.apply(l, r, {Z(b), Z(c)}); }
  i64 sum(int l, int r) { return t.prod(l, r).val(); }
};
