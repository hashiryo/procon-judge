#pragma once
#include "common.hpp"
#include "mylib/data_structure/KDTree.hpp"

// kd 木。点を座標で交互に 2 分していく 1 本の木で、長方形の最小は O(sqrt N)。
// 範囲は両端を含む形で受け取り、空の範囲は受け付けない (assert で落ちる) ので、
// 半開区間から直すときに空を先に除く。
struct Index {
  KDTree<int, 2, RangeMin> kdt;

  explicit Index(const vector<array<int, 3>> &xyw) : kdt(xyw) {}

  int min_in(int x0, int x1, int y0, int y1) {
    if (x0 >= x1 || y0 >= y1) return RangeMin::ti();
    return kdt.prod_cuboid(x0, x1 - 1, y0, y1 - 1);
  }
};

using Solver = DetourSolver<Index>;
