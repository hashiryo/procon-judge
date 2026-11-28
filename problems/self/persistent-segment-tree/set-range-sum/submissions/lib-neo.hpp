#pragma once
// NeoLibrary の永続セグメント木 (neo/base/ds/range/PersistentSegmentTree.hpp) を使う。
#include <cstddef>
#include "pj.hpp"
#include "neo/base/ds/range/PersistentSegmentTree.hpp"
#include "neo/base/ds/range/segtree/Sum.hpp"

struct Solver {
  using PST = PersistentSegmentTree<monoid::Sum<i64>>;
  std::vector<PST> ver;
  explicit Solver(const std::vector<i64> &a, int q) {
    // 初期の木 (約 2n 節) と、1 回の set で増える道 (log2(n) + 1 節) の分を先に取る。
    PST::reserve(std::size_t(a.size()) + std::size_t(q) * 20 + 1);
    ver.reserve(q + 1);
    ver.push_back(PST(a));
  }
  void set(int v, int i, i64 x) { ver.push_back(ver[v].set(i, x)); }
  i64 sum(int v, int l, int r) const { return ver[v].prod(l, r); }
};
