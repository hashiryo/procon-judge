#pragma once
#include "pj.hpp"
#include "mylib/data_structure/SegmentTree.hpp"

// 自分のライブラリの SegmentTree。segtree.hpp が同じ構造の手書き。
// このヘッダは mylib/internal/detection_idiom.hpp を include しているので、
// 閉包がライブラリの中で 2 段になる。
struct Solver {
  struct RangeSum {
    using T = i64;
    static T ti() { return 0; }
    static T op(T a, T b) { return a + b; }
  };

  SegmentTree<RangeSum> seg;

  explicit Solver(const vector<i64> &init) : seg(init) {}

  // mul は set(i, op(get(i), x))。点加算はこれで書く。
  void add(int p, i64 x) { seg.mul(p, x); }

  i64 sum(int l, int r) { return seg.prod(l, r); }
};
