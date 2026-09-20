#pragma once
#include "common.hpp"
#include "mylib/data_structure/KDTree.hpp"

// kd 木。x と y を交互に見て空間を二分する。持つものは点 1 つにつき 1 ノード
// だけで済むが、矩形の取得は最悪 O(sqrt N) になる。
struct Solver {
  KDTree<i64, 2, RangeSum> kdt;

  Solver(const vector<array<i64, 3>> &points, const vector<array<int, 2>> &spots)
      : kdt(to_map(points, spots)) {}

  void add(int x, int y, i64 w) { kdt.mul(x, y, w); }

  // kd 木の側は閉区間で受ける。
  i64 rect_sum(int l, int d, int r, int u) {
    return kdt.prod_cuboid(l, r - 1, d, u - 1);
  }
};
