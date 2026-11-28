#pragma once
#include "pj.hpp"
#include "_shared/kinetic/sqrt_hull.hpp"

// 参照実装。平方分割と塊ごとの凸包。計算量の保証があり、a の符号が入れ替わっても壊れない。
struct Solver {
  SqrtHull<true> s;
  explicit Solver(const vector<i64> &x, const vector<i64> &y) { s.init(x, y); }
  void add(int l, int r, i64 a, i64 b) { s.add(l, r, a, b); }
  void set(int i, i64 x, i64 y) { s.set(i, x, y); }
  i64 maximum(int l, int r) { return s.get(l, r); }
};
