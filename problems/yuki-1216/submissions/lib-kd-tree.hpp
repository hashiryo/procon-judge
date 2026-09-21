#pragma once
#include "common.hpp"
#include "mylib/data_structure/KDTree.hpp"

// kd 木。点を座標で交互に 2 分していく 1 本の木で、長方形の総和は O(sqrt N)。
// 1 点更新は根までの O(log N)。
struct Engine {
  KDTree<i64, 2, RangeCount> kdt;

  explicit Engine(const set<array<i64, 2>> &points) : kdt(points) {}

  void add(i64 x, i64 y, int delta) { kdt.mul(x, y, delta); }

  int count(i64 x0, i64 x1, i64 ymax) { return kdt.prod_cuboid(x0, x1 - 1, 0, ymax); }
};

using Solver = LanternSolver<Engine>;
