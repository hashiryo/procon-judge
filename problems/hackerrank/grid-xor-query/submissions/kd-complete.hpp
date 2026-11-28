#pragma once
#include "common.hpp"
#include "neo/base/ds/kd/KdTree.hpp"

// KD 木 (neo/base/ds/kd/KdTree.hpp、1 点 1 ノード) で、静的な点集合の長方形の XOR を取る。
// 今の Library の KDTree を使う提出 (lib-kd-tree.hpp) と書き比べる。
struct Index {
  KdTree<int, 2, RangeXor> kd;

  explicit Index(const vector<array<int, 3>> &pts) {
    vector<array<int, 2>> pt(pts.size());
    vector<int> v(pts.size());
    for (size_t i = 0; i < pts.size(); ++i) pt[i] = {pts[i][0], pts[i][1]}, v[i] = pts[i][2];
    kd = KdTree<int, 2, RangeXor>(pt, v);
  }

  int query(int a, int b, int c, int d) { return kd.prod({a, c}, {b + 1, d + 1}); }
};

using Solver = GridSolver<Index>;
