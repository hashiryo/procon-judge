#pragma once
#include "common.hpp"

// 愚直解 (提出ではない)。参照実装と小さい入力で突き合わせる。
struct Solver {
  vector<Mat> a;
  explicit Solver(const vector<Mat> &v) : a(v) {}
  void insert(int i, const Mat &m) { a.insert(a.begin() + i, m); }
  void erase(int i) { a.erase(a.begin() + i); }
  void reverse(int l, int r) { std::reverse(a.begin() + l, a.begin() + r); }
  Mat prod(int l, int r) {
    Mat s = MatMul::ti();
    for (int i = l; i < r; ++i) s = mat_mul(s, a[i]);
    return s;
  }
};
