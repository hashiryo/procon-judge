#pragma once
#include "common.hpp"
#include "_shared/kd-tree/kd_tree_2d.hpp"

// KD 木 (点を葉に置く完全 2 分木) で、静的な点集合の長方形の最小を取る。
// 今の Library の KDTree を使う提出 (lib-kd-tree.hpp) と書き比べる。
struct Index {
  KdTree2D<RangeMin> kd;

  explicit Index(const vector<array<int, 3>> &xyw) {
    vector<int> x(xyw.size()), y(xyw.size()), v(xyw.size());
    for (size_t i = 0; i < xyw.size(); ++i) x[i] = xyw[i][0], y[i] = xyw[i][1], v[i] = xyw[i][2];
    kd = KdTree2D<RangeMin>(x, y, v);
  }

  int min_in(int x0, int x1, int y0, int y1) { return kd.prod(x0, y0, x1, y1); }
};

using Solver = DetourSolver<Index>;
