#pragma once
#include "common.hpp"
#include "mylib/data_structure/KDTree.hpp"

// 2 次元の kd 木に Z の降順で生徒を足しながら、右上の領域の総和を問い合わせる。
// 1 点更新は O(log N)、領域の総和は O(sqrt N)。
struct Engine {
  KDTree<int, 2, RangeCount> kdt;

  explicit Engine(const set<array<int, 2>> &points) : kdt(points) {}

  void add(int x, int y) { kdt.mul(x, y, 1); }

  int count(int x, int y) { return kdt.prod_cuboid(x, INF, y, INF); }
};

using Solver = OfflineSolver<Engine>;
