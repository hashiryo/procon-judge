#pragma once
#include "pj.hpp"
#include <limits>
#include "_shared/segtree/_generic_flagbranch.hpp"

// 直した汎用の二分木の写しで、子に作用を当てるときの印の書き込みだけを直す前の形 (子の印で分岐する) に戻したもの。neo-generic と同じ自前の構造体を渡す。
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
