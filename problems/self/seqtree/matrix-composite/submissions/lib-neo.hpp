#pragma once
#include "common.hpp"
#include "neo/base/ds/seq/WeightBalancedTree.hpp"

// NeoLibrary の neo/base/ds/seq/WeightBalancedTree.hpp (永続しない形)。op が重く可換でないので、塊の形の Sequence でなく、
// 1 節に 1 値の重み平衡木を使う。
struct Solver {
  WeightBalancedTree<MatMul, true> t;
  explicit Solver(const vector<Mat> &a) : t(a) {}
  void insert(int i, const Mat &m) { t.insert(i, m); }
  void erase(int i) { t.erase(i); }
  void reverse(int l, int r) { t.reverse(l, r); }
  Mat prod(int l, int r) { return t.prod(l, r); }
};
