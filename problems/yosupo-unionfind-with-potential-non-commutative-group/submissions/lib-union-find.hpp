#pragma once
#include "common.hpp"
#include "mylib/data_structure/UnionFind_Potentialized.hpp"

// 経路圧縮つきの重み付き Union-Find。重みが行列なので、畳むたびに 2 次の
// 行列積が要る。
struct Solver {
  UnionFind_Potentialized<G> uf;

  explicit Solver(int n) : uf(n) {}

  int unite(int u, int v, const array<i64, 4> &x) {
    return uf.unite(u, v, to_mat(x));
  }

  bool diff(int u, int v, array<i64, 4> &out) {
    if (!uf.connected(u, v)) return false;
    return from_mat(uf.diff(u, v).x, out), true;
  }
};
