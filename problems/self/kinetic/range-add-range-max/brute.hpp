#pragma once
#include "pj.hpp"

// 愚直解 (提出ではない)。参照実装と小さい入力で突き合わせる。
struct Solver {
  vector<i64> x, y;
  explicit Solver(const vector<i64> &xs, const vector<i64> &ys) : x(xs), y(ys) {}
  void add(int l, int r, i64 a, i64 b) {
    for (int i = l; i < r; ++i) y[i] += a * x[i] + b;
  }
  void set(int i, i64 nx, i64 ny) { x[i] = nx, y[i] = ny; }
  i64 maximum(int l, int r) {
    i64 res = y[l];
    for (int i = l + 1; i < r; ++i) res = max(res, y[i]);
    return res;
  }
};
