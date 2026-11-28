#pragma once
#include "pj.hpp"
#include "neo/algo/optimize/monge/min_plus_convolution.hpp"

// NeoLibrary の min_plus_convolution_convex_convex (凸どうし) をそのまま使う。中身は差分の併合。
struct Solver {
  vector<i64> a, b, c;

  Solver(const vector<i64> &a, const vector<i64> &b) : a(a), b(b) {}

  void run() { c = min_plus_convolution_convex_convex(a, b); }

  const vector<i64> &answer() const { return c; }
};
