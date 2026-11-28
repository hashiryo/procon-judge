#pragma once
#include "common.hpp"
#include "_shared/kd-tree/kd_tree_2d.hpp"

// KD 木 (点を葉に置く完全 2 分木) で、静的な点集合の長方形の XOR を取る。範囲は両端を含む。
// 今の Library の KDTree を使う提出 (lib-kd-tree.hpp) と書き比べる。
struct Index {
  KdTree2D<RangeXor> kd;

  explicit Index(const vector<array<int, 3>> &pts) {
    vector<int> x(pts.size()), y(pts.size()), v(pts.size());
    for (size_t i = 0; i < pts.size(); ++i) x[i] = pts[i][0], y[i] = pts[i][1], v[i] = pts[i][2];
    kd = KdTree2D<RangeXor>(x, y, v);
  }

  int query(int a, int b, int c, int d) { return kd.prod(a, c, b + 1, d + 1); }
};

using Solver = GridSolver<Index>;
