#pragma once
#include "_seg_mont.hpp"

// 非再帰のセグメント木を Montgomery で。区間の取得は query_bl_split4 (_seg_mont.hpp)。
struct Solver {
  SegMont s;
  explicit Solver(const vector<array<i64, 2>> &f) : s(f) {}
  void set(int p, i64 c, i64 d) { s.set(p, c, d); }
  i64 query(int l, int r, i64 x) { return s.query_bl_split4(l, r, x); }
};
