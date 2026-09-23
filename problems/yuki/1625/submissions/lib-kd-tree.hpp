#pragma once
#include "common.hpp"
#include "mylib/data_structure/KDTree.hpp"

// kd 木。点を座標で交互に 2 分していく 1 本の木で、長方形の最大は O(sqrt N)。
// 1 点更新は根までの O(log N)。
struct Engine {
  KDTree<int, 2, RangeMax> kdt;

  explicit Engine(const map<array<int, 2>, i64> &points) : kdt(points) {}

  void mul(int l, int r, i64 s) { kdt.mul(l, r, s); }

  i64 max_in(int l, int r) { return kdt.prod_cuboid(l, r, l, r); }
};

using Solver = TriangleSolver<Engine>;
