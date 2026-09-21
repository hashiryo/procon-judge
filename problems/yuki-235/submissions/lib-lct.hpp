#pragma once
#include "common.hpp"
#include "mylib/data_structure/LinkCutTree.hpp"

// Link-Cut 木。道への作用も道の和も expose だけで済むので、HLD のような区間への
// 分解が要らない。和は可換なので、反転したときの値を別に持たなくてよい。
struct Solver {
  LinkCutTree<Inflation> lct;

  Solver(int n, const vector<i64> &s, const vector<i64> &c,
         const vector<array<int, 2>> &edges)
      : lct(n) {
    for (int i = 0; i < n; ++i) lct.set(i, {Mint(s[i]), Mint(c[i])});
    for (auto &e : edges) lct.link(e[0], e[1]);
  }

  void parade(int x, int y, i64 z) { lct.apply(x, y, Mint(z)); }

  i64 travel(int x, int y) { return lct.prod(x, y).s.val(); }
};
