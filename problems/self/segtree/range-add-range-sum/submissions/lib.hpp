#pragma once
#include "pj.hpp"
#include "mylib/data_structure/SegmentTree.hpp"

// 今の Library の SegmentTree に、和のモノイドと加算の作用 (区間の長さを受ける mp) を載せる。
struct Solver {
  struct RaddSum {
    using T = i64;
    using E = i64;
    static T ti() { return 0; }
    static T op(T a, T b) { return a + b; }
    static void mp(T &v, E x, int sz) { v += x * sz; }
    static void cp(E &pre, E suf) { pre += suf; }
  };
  SegmentTree<RaddSum> seg;
  explicit Solver(const vector<i64> &a) : seg(a) {}
  void add(int l, int r, i64 x) { seg.apply(l, r, x); }
  i64 sum(int l, int r) { return seg.prod(l, r); }
};
