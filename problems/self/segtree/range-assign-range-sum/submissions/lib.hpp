#pragma once
#include "pj.hpp"
#include <limits>
#include "mylib/data_structure/SegmentTree.hpp"

// 今の Library の SegmentTree に、代入の作用を自前の構造体で載せる。
struct Solver {
  struct M {
    using T = i64;
    using E = i64;
    static T ti() { return 0; }
    static T op(T a, T b) { return a + b; }
    static void mp(T &v, E x, int len) { v = x * len; }
    static void cp(E &pre, E suf) { pre = suf; }
  };
  SegmentTree<M> seg;
  explicit Solver(const vector<i64> &a) : seg(a) {}
  void assign(int l, int r, i64 x) { seg.apply(l, r, x); }
  i64 sum(int l, int r) { return seg.prod(l, r); }
};
