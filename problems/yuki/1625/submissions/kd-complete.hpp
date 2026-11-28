#pragma once
#include "common.hpp"
#include "_shared/kd-tree/kd_tree_2d.hpp"

// KD 木 (点を葉に置く完全 2 分木) で、点の max 更新と長方形の最大を取る。
// 今の Library の KDTree を使う提出 (lib-kd-tree.hpp) と書き比べる。
struct Engine {
  map<array<int, 2>, int> id;
  KdTree2D<RangeMax, i64> kd;

  explicit Engine(const map<array<int, 2>, i64> &points) {
    vector<int> x, y;
    vector<i64> v;
    x.reserve(points.size());
    y.reserve(points.size());
    v.reserve(points.size());
    for (const auto &e : points) {
      id[e.first] = (int)x.size();
      x.push_back(e.first[0]);
      y.push_back(e.first[1]);
      v.push_back(e.second);
    }
    kd = KdTree2D<RangeMax, i64>(x, y, v);
  }

  void mul(int l, int r, i64 s) { kd.mul(id[{l, r}], s); }

  i64 max_in(int l, int r) { return kd.prod(l, l, r + 1, r + 1); }
};

using Solver = TriangleSolver<Engine>;
