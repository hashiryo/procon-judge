#pragma once
#include "pj.hpp"
#include "neo/data_structure/RangeLinearAddRangeMin.hpp"

// NeoLibrary の RangeLinearAddRangeMin。kinetic_bridge を写したもので、橋を探し直すときの外積をすべて 128 bit で取る。
struct Solver {
  RangeLinearAddRangeMin s;
  explicit Solver(const vector<i64> &a) : s(a) {}
  void add(int l, int r, i64 b, i64 c) { s.add(l, r, b, c); }
  i64 minimum(int l, int r) { return s.min(l, r); }
};
