#pragma once
#include "common.hpp"
#include "mylib/data_structure/UnionFind_Potentialized_Undoable.hpp"

// 巻き戻せる重み付き Union-Find。履歴を残すために経路圧縮ができないので、
// 1 クエリは O(log N) になる。この問題では巻き戻しを使わないので、経路圧縮を
// 捨てたぶんがそのまま差になる。
struct Solver {
  UnionFind_Potentialized_Undoable<Mint> uf;

  explicit Solver(int n) : uf(n) {}

  int unite(int u, int v, i64 x) { return uf.unite(u, v, Mint(x)); }

  i64 diff(int u, int v) {
    return uf.connected(u, v) ? (i64)uf.diff(u, v).val() : -1;
  }
};
