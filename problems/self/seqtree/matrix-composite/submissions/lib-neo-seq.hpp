#pragma once
#include "common.hpp"
#include "neo/data_structure/Sequence.hpp"

// NeoLibrary の neo/data_structure/Sequence.hpp (塊の形)。lib-neo (1 節に 1 値の WeightBalancedTree) と比べるために置く。op が重く
// 可換でない列を反転すると、塊の順と逆順の積を塊の長さだけ求め直すので、反転の多いケースで遅くなる。
struct Solver {
  Sequence<MatMul, true> t;
  explicit Solver(const vector<Mat> &a) : t(a) {}
  void insert(int i, const Mat &m) { t.insert(i, m); }
  void erase(int i) { t.erase(i); }
  void reverse(int l, int r) { t.reverse(l, r); }
  Mat prod(int l, int r) { return t.prod(l, r); }
};
