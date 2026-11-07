#pragma once
#include "pj.hpp"
#include <limits>
#include "neo/data_structure/SegmentTree.hpp"

// NeoLibrary の SegmentTree に自前の最小のモノイドを渡す。標準の構造体ではないので、中身は二分木の汎用の形。
// 標準の構造体 monoid::Min を渡す lib-neo と比べるための提出。
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
