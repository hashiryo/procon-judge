#pragma once
#include "_path.hpp"
#include "_wide.hpp"

// 無向の辺から HLD を隣の番号の xor で葉から剥がして作り (中身は _path.hpp)、道を位置の区間に分けて、節を 8 個に
// 広げた接頭辞和の木 (_wide.hpp、point-add-range-sum の wide8 の写し) で区間ごとの和を取る。
struct Solver {
  path_hld::Tree t;
  WidePrefixSum<3> w;
  static vector<i64> by_pos(const path_hld::Tree &t, const vector<i64> &a) {
    vector<i64> v(t.n);
    for (int u = 0; u < t.n; ++u) v[t.pos[u]] = a[u];
    return v;
  }
  Solver(int n, const vector<i64> &a, const vector<array<int, 2>> &edges)
      : t(path_hld::Tree::make<path_hld::Build::Xor>(n, edges)), w(by_pos(t, a)) {}
  void add(int p, i64 x) { w.add(t.pos[p], x); }
  i64 path_sum(int u, int v) {
    i64 s = 0;
    t.path_unordered(u, v, [&](int l, int r) { s += w.sum(l, r + 1); });
    return s;
  }
};
