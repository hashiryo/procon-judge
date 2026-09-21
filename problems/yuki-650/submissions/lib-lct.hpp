#pragma once
#include "common.hpp"
#include "mylib/data_structure/LinkCutTree.hpp"

// Link-Cut 木。辺を頂点にして両端の間に挟み、道の積を expose だけで出す。
// 積が可換でないので、構造の側が反転したときの積を別に持つ。
struct Solver {
  int n;
  LinkCutTree<MatProd> lct;

  Solver(int n, const vector<array<int, 2>> &edges) : n(n), lct(n + n - 1, IDENTITY) {
    for (int i = 0; i < n - 1; ++i) {
      lct.link(edges[i][0], n + i);
      lct.link(n + i, edges[i][1]);
    }
  }

  void set(int e, const array<i64, 4> &x) { lct.set(n + e, to_mat(x)); }

  array<i64, 4> product(int u, int v) { return from_mat(lct.prod(u, v)); }
};
