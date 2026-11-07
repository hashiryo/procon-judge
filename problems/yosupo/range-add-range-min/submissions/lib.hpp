#pragma once
#include "pj.hpp"
#include <limits>
#include "mylib/data_structure/SegmentTree.hpp"

// 今の Library の SegmentTree に、最小値のモノイドと加算の作用を載せる。
struct Solver {
  struct RaddMin {
    using T = i64;
    using E = i64;
    static T ti() { return numeric_limits<i64>::max(); }
    static T op(T l, T r) { return l < r ? l : r; }
    static void mp(T &v, E x) { v += x; }
    static void cp(E &pre, E suf) { pre += suf; }
  };
  SegmentTree<RaddMin> seg;

  explicit Solver(const vector<i64> &a) : seg(a) {}
  void add(int l, int r, i64 x) { seg.apply(l, r, x); }
  i64 minimum(int l, int r) { return seg.prod(l, r); }
};
