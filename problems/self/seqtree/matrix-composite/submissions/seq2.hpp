#pragma once
#include "common.hpp"
#include "_shared/seqtree/seq2.hpp"

// NeoLibrary の Sequence (塊の形) の手直しの試作 (problems/_shared/seqtree/seq2.hpp)。lib-neo-seq (今の Sequence) と比べる。
struct Solver {
  Seq2<MatMul, true> t;
  explicit Solver(const vector<Mat> &a) : t(a) {}
  void insert(int i, const Mat &m) { t.insert(i, m); }
  void erase(int i) { t.erase(i); }
  void reverse(int l, int r) { t.reverse(l, r); }
  Mat prod(int l, int r) { return t.prod(l, r); }
};
