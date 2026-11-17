#pragma once
#include "pj.hpp"
#include <limits>
#define GENERIC_EXP_ALLOC 2
#include "_shared/segtree/_generic_exp.hpp"

// NeoLibrary 1a83046 の汎用の二分木の写し (_generic_exp.hpp) で、配列は 2 MB 境界と huge page のまま、lz と fl の先頭だけを 2 KB と 64 byte ずらした形 (d[k] と lz[k] の下位 21 bit をそろえない)。neo-generic と同じ自前の構造体を渡す。
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
