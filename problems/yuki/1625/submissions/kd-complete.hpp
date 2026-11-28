#pragma once
#include "common.hpp"
#include "neo/base/ds/kd/KdTree.hpp"

// KD 木 (neo/base/ds/kd/KdTree.hpp、1 点 1 ノード) で、点の max 更新と長方形の最大を取る。
// 今の Library の KDTree を使う提出 (lib-kd-tree.hpp) と書き比べる。
struct Engine {
  map<array<int, 2>, int> id;
  KdTree<int, 2, RangeMax> kd;

  explicit Engine(const map<array<int, 2>, i64> &points) {
    vector<array<int, 2>> pt;
    vector<i64> v;
    pt.reserve(points.size());
    v.reserve(points.size());
    for (const auto &e : points) {
      id[e.first] = (int)pt.size();
      pt.push_back(e.first);
      v.push_back(e.second);
    }
    kd = KdTree<int, 2, RangeMax>(pt, v);
  }

  void mul(int l, int r, i64 s) { kd.mul(id[{l, r}], s); }

  i64 max_in(int l, int r) { return kd.prod({l, l}, {r + 1, r + 1}); }
};

using Solver = TriangleSolver<Engine>;
