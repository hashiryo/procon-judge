#pragma once
#include "pj.hpp"
#include "mylib/data_structure/LinkCutTree.hpp"

// Link-Cut 木。道の総和を expose だけで出せるので、HLD のような区間への
// 分解が要らない。和は可換なので、反転したときの総和を別に持たなくてよい。
struct Solver {
  struct RangeSum {
    using T = i64;
    static T op(T l, T r) { return l + r; }
    using commute = void;
  };

  LinkCutTree<RangeSum> lct;

  Solver(int n, const vector<i64> &a, const vector<array<int, 2>> &edges)
      : lct(n) {
    for (int i = 0; i < n; ++i) lct.set(i, a[i]);
    for (auto &e : edges) lct.link(e[0], e[1]);
  }

  void add(int p, i64 x) { lct.mul(p, x); }

  i64 path_sum(int u, int v) { return lct.prod(u, v); }
};
