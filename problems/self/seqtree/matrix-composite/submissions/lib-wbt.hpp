#pragma once
#include "common.hpp"
#include "mylib/data_structure/WeightBalancedTree.hpp"

// 今の Library の重み平衡木 (値を葉に持つ)。可換でないモノイドを反転するので、逆順の積も持つ。
struct Solver {
  WeightBalancedTree<MatMul, true> t;
  explicit Solver(const vector<Mat> &a) : t(a) {}
  void insert(int i, const Mat &m) { t.insert(i, m); }
  void erase(int i) { t.erase(i); }
  void reverse(int l, int r) { t.reverse(l, r); }
  Mat prod(int l, int r) { return t.prod(l, r); }
};
