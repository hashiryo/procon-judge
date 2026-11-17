#pragma once
#include "pj.hpp"
#include <limits>
#define GENERIC_EXP_SEQ 1
#include "_shared/segtree/_generic_exp.hpp"

// NeoLibrary 1a83046 の汎用の二分木の写し (_generic_exp.hpp) で、両端の道を配るのと apply のあとの求め直しを、今の Library と同じく l の道、r の道の順に、切れる段だけ行う形。neo-generic と同じ自前の構造体を渡す。
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
