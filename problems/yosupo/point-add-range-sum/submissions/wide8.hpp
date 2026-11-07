#pragma once
#include "_wide.hpp"

// 節を 8 個に広げた接頭辞和の木。中身は _wide.hpp。
struct Solver {
  WidePrefixSum<3> w;
  explicit Solver(const vector<i64> &a) : w(a) {}
  void add(int p, i64 x) { w.add(p, x); }
  i64 sum(int l, int r) { return w.sum(l, r); }
};
