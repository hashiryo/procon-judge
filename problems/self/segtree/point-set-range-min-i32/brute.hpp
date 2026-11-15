#pragma once
#include "pj.hpp"

// 愚直解 (提出ではない)。参照実装と小さい入力で突き合わせる。
struct Solver {
  vector<i64> a;
  explicit Solver(const vector<i64> &v) : a(v) {}
  void set(int p, i64 x) { a[p] = x; }
  i64 minimum(int l, int r) {
    i64 res = a[l];
    for (int i = l; i < r; ++i) res = min(res, a[i]);
    return res;
  }
};
