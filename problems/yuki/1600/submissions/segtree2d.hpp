#pragma once
#include "common.hpp"
#include "neo/base/ds/2d/SegTree2D.hpp"

// モノイドを受ける 2 次元セグメント木 (merge sort 木) の候補で、長方形の最小を取る。
// 今の Library の SegmentTree_2D を使う提出 (lib-segtree-2d.hpp) と書き比べる。
struct Index {
  SegTree2D<RangeMin> st;

  explicit Index(const vector<array<int, 3>> &xyw) {
    vector<int> x(xyw.size()), y(xyw.size()), v(xyw.size());
    for (size_t i = 0; i < xyw.size(); ++i) x[i] = xyw[i][0], y[i] = xyw[i][1], v[i] = xyw[i][2];
    st = SegTree2D<RangeMin>(x, y, v);
  }

  int min_in(int x0, int x1, int y0, int y1) { return st.prod(x0, y0, x1, y1); }
};

using Solver = DetourSolver<Index>;
