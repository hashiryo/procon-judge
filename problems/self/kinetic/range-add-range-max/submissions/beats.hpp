#pragma once
#include "pj.hpp"
#include "_shared/kinetic/beats.hpp"

// kinetic segment tree (Segment Tree Beats の形)。作用の a が 0 以上でないと使えない。
struct Solver {
  KineticBeats<true> s;
  explicit Solver(const vector<i64> &x, const vector<i64> &y) { s.init(x, y); }
  void add(int l, int r, i64 a, i64 b) { s.add(l, r, a, b); }
  void set(int i, i64 x, i64 y) { s.set(i, x, y); }
  i64 maximum(int l, int r) { return s.get(l, r); }
};
