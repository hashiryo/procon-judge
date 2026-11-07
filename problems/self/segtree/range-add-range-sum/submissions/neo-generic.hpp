#pragma once
#include "pj.hpp"
#include "neo/data_structure/SegmentTree.hpp"

// NeoLibrary の SegmentTree に自前の和と加算を渡す。標準の構造体ではないので、中身は二分木の汎用の遅延セグ木。
// 標準の構造体 monoid::AddSum を渡す lib-neo と比べるための提出。
struct Solver {
  struct RaddSum {
    using T = i64;
    using E = i64;
    static T ti() { return 0; }
    static T op(T a, T b) { return a + b; }
    static void mp(T &v, E x, int len) { v += x * len; }
    static void cp(E &pre, E suf) { pre += suf; }
  };
  SegmentTree<RaddSum> seg;
  explicit Solver(const vector<i64> &a) : seg(a) {}
  void add(int l, int r, i64 x) { seg.apply(l, r, x); }
  i64 sum(int l, int r) { return seg.prod(l, r); }
};
