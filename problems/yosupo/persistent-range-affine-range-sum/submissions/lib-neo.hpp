#pragma once
// NeoLibrary の永続遅延セグメント木 (neo/base/ds/range/PersistentLazySegmentTree.hpp) に
// monoid::AffineSum<ZMod<998244353>> を渡す。apply と copy_range が新しい版を返す。
#include <cstddef>
#include "pj.hpp"
#include "neo/base/core/algebra/ZMod.hpp"
#include "neo/base/ds/range/PersistentLazySegmentTree.hpp"
#include "neo/base/ds/range/segtree/Affine.hpp"

struct Solver {
  static constexpr int MOD = 998244353;
  using Z = ZMod<MOD>;
  using PLST = PersistentLazySegmentTree<monoid::AffineSum<Z>>;
  vector<PLST> ver;
  explicit Solver(const vector<int> &a, int q) : ver(q + 1) {
    // copy_range は的の道と元の道の両方を写すので、1 回あたり 4 log n 節ほど増える。
    PLST::reserve(std::size_t(a.size()) * 2 + std::size_t(q) * 80 + 1);
    vector<Z> z(a.size());
    for (std::size_t i = 0; i < a.size(); ++i) z[i] = Z(a[i]);
    ver[0] = PLST(z);
  }
  void apply(int i, int k, int l, int r, int b, int c) {
    ver[i + 1] = ver[k + 1].apply(l, r, {Z(b), Z(c)});
  }
  void copy(int i, int k, int s, int l, int r) {
    ver[i + 1] = ver[k + 1].copy_range(l, r, ver[s + 1]);
  }
  int sum(int k, int l, int r) const { return int(ver[k + 1].prod(l, r).val()); }
};
