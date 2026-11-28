#pragma once
#include "pj.hpp"
#include "neo/base/ds/range/KineticSegmentTree.hpp"

// NeoLibrary の KineticSegmentTree (仮名) で、時刻の前進を全区間への a の加算にする形。
struct Solver {
  KineticSegmentTree<true> s;
  int n;
  i64 now = 0;
  explicit Solver(const vector<i64> &a, const vector<i64> &b) : s(a, b), n(int(a.size())) {}
  void set(int i, i64 a, i64 b) { s.set(i, a, a * now + b); }
  i64 query(i64 t) {
    if (t > now) s.add(0, n, t - now, 0), now = t;
    return s.all_prod();
  }
};
