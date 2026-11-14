#pragma once
#include "pj.hpp"
#include <limits>
#include "_shared/segtree/_generic_a70dab2.hpp"

// NeoLibrary a70dab2 の汎用の二分木 (直す前の形) の写しに、neo-generic と同じ自前の構造体を渡す。直した形と同じ回で比べるために置く。
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
