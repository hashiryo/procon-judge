#pragma once
#include "pj.hpp"
#include "neo/algebra/ZMod.hpp"
#include "_shared/seqtree/seq2.hpp"

// NeoLibrary の Sequence (塊の形) の手直しの試作 (problems/_shared/seqtree/seq2.hpp)。塊は 512 で、作るときは 128 ずつ。
struct Solver {
  using Z = ZMod<998244353>;
  Seq2<monoid::AffineSum<Z>, true> t;
  explicit Solver(const vector<i64> &a) : t(vector<Z>(a.begin(), a.end())) {}
  void insert(int i, i64 x) { t.insert(i, Z(x)); }
  void erase(int i) { t.erase(i); }
  void reverse(int l, int r) { t.reverse(l, r); }
  void affine(int l, int r, i64 b, i64 c) { t.apply(l, r, {Z(b), Z(c)}); }
  i64 sum(int l, int r) { return t.prod(l, r).val(); }
};
