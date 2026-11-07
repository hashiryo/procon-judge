#pragma once
#include "pj.hpp"
#include <limits>
#include "mylib/data_structure/SegmentTree.hpp"

// 今の Library の SegmentTree に最小のモノイドを載せる。
struct Solver {
  struct RminQ {
    using T = i64;
    static T ti() { return numeric_limits<i64>::max(); }
    static T op(T a, T b) { return a < b ? a : b; }
  };
  SegmentTree<RminQ> seg;
  explicit Solver(const vector<i64> &a) : seg(a) {}
  void set(int p, i64 x) { seg.set(p, x); }
  i64 minimum(int l, int r) { return seg.prod(l, r); }
};
