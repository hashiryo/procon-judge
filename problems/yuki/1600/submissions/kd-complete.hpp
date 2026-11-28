#pragma once
#include "common.hpp"
#include "neo/base/ds/kd/KdTree.hpp"

// KD 木 (neo/base/ds/kd/KdTree.hpp、1 点 1 ノード) で、静的な点集合の長方形の最小を取る。
// 今の Library の KDTree を使う提出 (lib-kd-tree.hpp) と書き比べる。
struct Index {
  KdTree<int, 2, RangeMin> kd;

  explicit Index(const vector<array<int, 3>> &xyw) {
    vector<array<int, 2>> pt(xyw.size());
    vector<int> v(xyw.size());
    for (size_t i = 0; i < xyw.size(); ++i) pt[i] = {xyw[i][0], xyw[i][1]}, v[i] = xyw[i][2];
    kd = KdTree<int, 2, RangeMin>(pt, v);
  }

  int min_in(int x0, int x1, int y0, int y1) { return kd.prod({x0, y0}, {x1, y1}); }
};

using Solver = DetourSolver<Index>;
