#pragma once
#include "pj.hpp"
#include "neo/algebra/ZMod.hpp"
#include "_shared/seqtree/seq3.hpp"

// NeoLibrary の Sequence (6320747) の塊の配列の間隔を 1 キャッシュ行延ばした試作 (problems/_shared/seqtree/seq3.hpp)。
struct Solver {
  using Z = ZMod<998244353>;
  Seq3<monoid::AffineSum<Z>, true> t;
  explicit Solver(const vector<i64> &a) : t(vector<Z>(a.begin(), a.end())) {}
  void insert(int i, i64 x) { t.insert(i, Z(x)); }
  void erase(int i) { t.erase(i); }
  void reverse(int l, int r) { t.reverse(l, r); }
  void affine(int l, int r, i64 b, i64 c) { t.apply(l, r, {Z(b), Z(c)}); }
  i64 sum(int l, int r) { return t.prod(l, r).val(); }
};
