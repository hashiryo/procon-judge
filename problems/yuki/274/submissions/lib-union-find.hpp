#pragma once
#include "common.hpp"
#include "mylib/data_structure/UnionFind_Potentialized.hpp"

// 経路圧縮つきのポテンシャル付き Union-Find。x_i xor x_j を辺の重みに持ち、
// 矛盾する unite が出た時点で止める。
struct Solver {
  int m;
  vector<i64> l, r;
  bool ok = false;

  Solver(int m, const vector<i64> &l, const vector<i64> &r) : m(m), l(l), r(r) {}

  void run() {
    const int n = (int)l.size();
    UnionFind_Potentialized<bool> uf(n);
    for (int i = 0; i < n; ++i)
      for (int j = 0; j < i; ++j) {
        auto [same, differ] = constraint(m, l[i], r[i], l[j], r[j]);
        if (differ && !uf.unite(i, j, true)) return;
        if (same && !uf.unite(i, j, false)) return;
      }
    ok = true;
  }

  bool answer() const { return ok; }
};
