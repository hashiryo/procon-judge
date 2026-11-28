#pragma once
#include "pj.hpp"
#include "_shared/kinetic/melt.hpp"

// イベントを使わない kinetic segment tree。時刻を前後に行き来させても動くが、計算量の保証は無い。
struct Solver {
  KineticMelt<true> s;
  explicit Solver(const vector<i64> &x, const vector<i64> &y) { s.init(x, y); }
  void add(int l, int r, i64 a, i64 b) { s.add(l, r, a, b); }
  void set(int i, i64 x, i64 y) { s.set(i, x, y); }
  i64 maximum(int l, int r) { return s.get(l, r); }
};
