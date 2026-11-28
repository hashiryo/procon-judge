#pragma once
#include "pj.hpp"
#include "neo/base/ds/range/KineticSegmentTree.hpp"

// NeoLibrary の KineticSegmentTree<false> (仮名) をそのまま使う形。
struct Solver {
  KineticSegmentTree<false> s;
  explicit Solver(const vector<i64> &x, const vector<i64> &y) : s(x, y) {}
  void add(int l, int r, i64 a, i64 b) { s.add(l, r, a, b); }
  void set(int i, i64 x, i64 y) { s.set(i, x, y); }
  i64 minimum(int l, int r) { return s.prod(l, r); }
};
