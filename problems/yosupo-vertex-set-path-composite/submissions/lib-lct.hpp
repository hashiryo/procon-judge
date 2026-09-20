#pragma once
#include "common.hpp"
#include "mylib/data_structure/LinkCutTree.hpp"

// Link-Cut 木。道の積を expose だけで出せる。合成が可換でないので、構造の側が
// 反転したときの積を別に持って、evert で向きが変わったときに入れ替える。
struct Solver {
  LinkCutTree<Composite> lct;

  Solver(int n, const vector<array<i64, 2>> &f,
         const vector<array<int, 2>> &edges)
      : lct(n) {
    for (int i = 0; i < n; ++i) lct.set(i, {Mint(f[i][0]), Mint(f[i][1])});
    for (auto &e : edges) lct.link(e[0], e[1]);
  }

  void set(int p, i64 c, i64 d) { lct.set(p, {Mint(c), Mint(d)}); }

  i64 composite(int u, int v, i64 x) {
    auto [a, b] = lct.prod(u, v);
    return (a * Mint(x) + b).val();
  }
};
