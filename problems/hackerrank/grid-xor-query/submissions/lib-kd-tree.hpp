#pragma once
#include "common.hpp"
#include "mylib/data_structure/KDTree.hpp"

// kd 木。点を座標で交互に 2 分していく 1 本の木で、長方形の XOR は O(sqrt N)。
// 範囲は両端を含む形で受け取る。
struct Index {
  KDTree<int, 2, RangeXor> kdt;

  explicit Index(const vector<array<int, 3>> &pts) : kdt(pts) {}

  int query(int a, int b, int c, int d) { return kdt.prod_cuboid(a, b, c, d); }
};

using Solver = GridSolver<Index>;
