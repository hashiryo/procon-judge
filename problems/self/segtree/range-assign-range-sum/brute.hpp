#pragma once
#include "pj.hpp"

// 愚直解 (提出ではない)。参照実装と小さい入力で突き合わせる。
struct Solver {
  vector<i64> a;
  explicit Solver(const vector<i64> &v) : a(v) {}
  void assign(int l, int r, i64 x) {
    for (int i = l; i < r; ++i) a[i] = x;
  }
  i64 sum(int l, int r) {
    i64 res = 0;
    for (int i = l; i < r; ++i) res += a[i];
    return res;
  }
};
