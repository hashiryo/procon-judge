#pragma once
// NeoLibrary の永続セグメント木 (neo/base/ds/range/PersistentSegmentTree.hpp) で区間 max xor を解く。
// 値 (30 bit) を位置とみなし、版 i を先頭 i 個を入れた木にする。2 版の差で max_xor を引く。
#include <cstddef>
#include "pj.hpp"
#include "neo/base/ds/range/PersistentSegmentTree.hpp"
#include "neo/base/ds/range/segtree/Sum.hpp"

struct Solver {
  using PST = PersistentSegmentTree<monoid::Sum<int>>;
  static constexpr int BITS = 30;
  vector<PST> ver;
  explicit Solver(const vector<int> &a) {
    PST::reserve(std::size_t(a.size()) * (BITS + 1) + 1);
    ver.reserve(a.size() + 1);
    ver.push_back(PST(1 << BITS));
    for (int x : a) ver.push_back(ver.back().mul(x, 1));
  }
  int max_xor(int l, int r, int x) const { return int(PST::max_xor(ver[l], ver[r], x)); }
};
