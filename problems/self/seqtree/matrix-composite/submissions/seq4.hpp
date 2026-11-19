#pragma once
#include "common.hpp"
#include "_shared/seqtree/seq4.hpp"

// NeoLibrary の Sequence (d8f0437) の区間の反転で、下りた道を下からたどって割る試作 (problems/_shared/seqtree/seq4.hpp)。lib-neo-seq と
// 比べる。
struct Solver {
  Seq4<MatMul, true> t;
  explicit Solver(const vector<Mat> &a) : t(a) {}
  void insert(int i, const Mat &m) { t.insert(i, m); }
  void erase(int i) { t.erase(i); }
  void reverse(int l, int r) { t.reverse(l, r); }
  Mat prod(int l, int r) { return t.prod(l, r); }
};
