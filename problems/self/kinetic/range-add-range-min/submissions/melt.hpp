#pragma once
#include "pj.hpp"
#include "_shared/kinetic/melt.hpp"

// イベントを使わない kinetic segment tree の最小側。a の符号は任意だが、計算量の保証は無い。
struct Solver {
  KineticMelt<false> s;
  explicit Solver(const vector<i64> &x, const vector<i64> &y) { s.init(x, y); }
  void add(int l, int r, i64 a, i64 b) { s.add(l, r, a, b); }
  void set(int i, i64 x, i64 y) { s.set(i, x, y); }
  i64 minimum(int l, int r) { return s.get(l, r); }
};
