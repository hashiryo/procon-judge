#pragma once
#include "common.hpp"
#include "_shared/segtree2d/segtree2d.hpp"

// モノイドを受ける 2 次元セグメント木 (merge sort 木) の候補で、長方形の XOR を取る。
// 今の Library の SegmentTree_2D を使う提出 (lib-segtree-2d.hpp) と書き比べる。
struct Index {
  SegTree2D<RangeXor> st;

  explicit Index(const vector<array<int, 3>> &pts) {
    vector<int> x(pts.size()), y(pts.size()), v(pts.size());
    for (size_t i = 0; i < pts.size(); ++i) x[i] = pts[i][0], y[i] = pts[i][1], v[i] = pts[i][2];
    st = SegTree2D<RangeXor>(x, y, v);
  }

  int query(int a, int b, int c, int d) { return st.prod(a, c, b + 1, d + 1); }
};

using Solver = GridSolver<Index>;
