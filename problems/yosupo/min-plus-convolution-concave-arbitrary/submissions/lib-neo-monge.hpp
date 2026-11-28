#pragma once
#include "pj.hpp"
#include "neo/algo/optimize/monge/min_plus_convolution.hpp"

// NeoLibrary の min_plus_convolution_concave (階段を長い辺で切る分割統治と、全部有効な矩形での
// 列の逆順 + monotone_minima) をそのまま使う。
struct Solver {
  vector<i64> a, b, c;

  Solver(const vector<i64> &a, const vector<i64> &b) : a(a), b(b) {}

  void run() { c = min_plus_convolution_concave(a, b); }

  const vector<i64> &answer() const { return c; }
};
