#pragma once
#include "common.hpp"
#include "mylib/data_structure/UnionFind_Potentialized_Undoable.hpp"

// 巻き戻せる重み付き Union-Find。履歴を残すために経路圧縮ができないので、
// 根まで登る距離が O(log N) になる。重みが行列なので、その距離ぶんの行列積が
// そのまま効く。
struct Solver {
  UnionFind_Potentialized_Undoable<G> uf;

  explicit Solver(int n) : uf(n) {}

  int unite(int u, int v, const array<i64, 4> &x) {
    return uf.unite(u, v, to_mat(x));
  }

  bool diff(int u, int v, array<i64, 4> &out) {
    if (!uf.connected(u, v)) return false;
    return from_mat(uf.diff(u, v).x, out), true;
  }
};
