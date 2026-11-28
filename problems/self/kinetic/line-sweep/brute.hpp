#pragma once
#include "pj.hpp"

// 愚直解 (提出ではない)。参照実装と小さい入力で突き合わせる。
struct Solver {
  vector<i64> a, b;
  explicit Solver(const vector<i64> &as, const vector<i64> &bs) : a(as), b(bs) {}
  void set(int i, i64 na, i64 nb) { a[i] = na, b[i] = nb; }
  i64 query(i64 t) {
    i64 res = -(i64(1) << 62);
    for (size_t i = 0; i < a.size(); ++i) res = max(res, a[i] * t + b[i]);
    return res;
  }
};
