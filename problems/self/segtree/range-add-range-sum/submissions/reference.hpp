#pragma once
#include "pj.hpp"

// 参照実装。差分を 2 本の Fenwick tree に持つ定番の形 (Library に依存しない)。
// [0, p) の和は S0[p] + p B1(p) - B2(p) で、B1 は差分 d[j]、B2 は d[j] j の接頭辞和。
struct Solver {
  int n;
  vector<i64> s0, b1, b2;
  explicit Solver(const vector<i64> &a) : n((int)a.size()), s0(a.size() + 1), b1(a.size() + 1), b2(a.size() + 1) {
    for (int i = 0; i < n; ++i) s0[i + 1] = s0[i] + a[i];
  }
  void upd(vector<i64> &b, int p, i64 x) {
    for (++p; p <= n; p += p & -p) b[p] += x;
  }
  i64 pre(const vector<i64> &b, int p) const {
    i64 s = 0;
    for (; p > 0; p -= p & -p) s += b[p];
    return s;
  }
  i64 prefix(int p) const { return s0[p] + i64(p) * pre(b1, p) - pre(b2, p); }
  void add(int l, int r, i64 x) {
    upd(b1, l, x), upd(b2, l, x * l);
    if (r < n) upd(b1, r, -x), upd(b2, r, -x * r);
  }
  i64 sum(int l, int r) { return prefix(r) - prefix(l); }
};
