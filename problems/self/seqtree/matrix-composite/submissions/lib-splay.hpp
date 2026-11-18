#pragma once
#include "common.hpp"
#include "mylib/data_structure/SplayTree.hpp"

// 今の Library のスプレー木。可換でないモノイドを反転するので、逆順の積も持つ。
struct Solver {
  SplayTree<MatMul, true> t;
  explicit Solver(const vector<Mat> &a) : t(a) {}
  void insert(int i, const Mat &m) { t.insert(i, m); }
  void erase(int i) { t.erase(i); }
  void reverse(int l, int r) { t.reverse(l, r); }
  Mat prod(int l, int r) { return t.prod(l, r); }
};
