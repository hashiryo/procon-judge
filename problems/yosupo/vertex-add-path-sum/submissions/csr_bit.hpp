#pragma once
#include "_path.hpp"

// 無向の辺から HLD を隣接の CSR を組んで BFS して作る (中身は _path.hpp)。道を位置の区間に分け、区間ごとに BIT の和を取る。
struct Solver {
  path_hld::Tree t;
  path_hld::Fenwick bit;
  static vector<i64> by_pos(const path_hld::Tree &t, const vector<i64> &a) {
    vector<i64> v(t.n);
    for (int u = 0; u < t.n; ++u) v[t.pos[u]] = a[u];
    return v;
  }
  Solver(int n, const vector<i64> &a, const vector<array<int, 2>> &edges)
      : t(path_hld::Tree::make<path_hld::Build::Csr>(n, edges)), bit(n, by_pos(t, a).data()) {}
  void add(int p, i64 x) { bit.add(t.pos[p], x); }
  i64 path_sum(int u, int v) {
    i64 s = 0;
    t.path_unordered(u, v, [&](int l, int r) { s += bit.sum(l, r + 1); });
    return s;
  }
};
