#pragma once
#include "pj.hpp"
#include <limits>
#include "mylib/data_structure/SegmentTree.hpp"

// 今の Library の SegmentTree に、代入の作用を自前の構造体で載せる。
struct Solver {
  struct M {
    using T = i64;
    using E = i64;
    static T ti() { return numeric_limits<i64>::max(); }
    static T op(T a, T b) { return a < b ? a : b; }
    static void mp(T &v, E x) { v = x; }
    static void cp(E &pre, E suf) { pre = suf; }
  };
  SegmentTree<M> seg;
  explicit Solver(const vector<i64> &a) : seg(a) {}
  void assign(int l, int r, i64 x) { seg.apply(l, r, x); }
  i64 minimum(int l, int r) { return seg.prod(l, r); }
};
