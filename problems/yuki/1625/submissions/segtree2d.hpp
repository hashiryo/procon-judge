#pragma once
#include "common.hpp"
#include "neo/base/ds/2d/SegTree2D.hpp"

// モノイドを受ける 2 次元セグメント木 (merge sort 木) の候補で、点の max 更新と
// 長方形の最大を取る。今の Library の SegmentTree_2D を使う提出と書き比べる。
struct Engine {
  map<array<int, 2>, int> id;
  SegTree2D<RangeMax, i64> st;

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
    st = SegTree2D<RangeMax, i64>(x, y, v);
  }

  void mul(int l, int r, i64 s) { st.mul(id[{l, r}], s); }

  i64 max_in(int l, int r) { return st.prod(l, l, r + 1, r + 1); }
};

using Solver = TriangleSolver<Engine>;
