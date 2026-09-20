#pragma once
#include "common.hpp"
#include "mylib/data_structure/UnionFind_Potentialized.hpp"

// 経路圧縮つきの重み付き Union-Find。潰しながら根までの重みを畳む。
struct Solver {
  UnionFind_Potentialized<Mint> uf;

  explicit Solver(int n) : uf(n) {}

  int unite(int u, int v, i64 x) { return uf.unite(u, v, Mint(x)); }

  i64 diff(int u, int v) {
    return uf.connected(u, v) ? (i64)uf.diff(u, v).val() : -1;
  }
};
