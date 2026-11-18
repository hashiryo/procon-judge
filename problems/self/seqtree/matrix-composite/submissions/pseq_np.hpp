#pragma once
#include "common.hpp"
#include "_shared/seqtree/persistent_seq.hpp"

// 永続の列の木の試作 (problems/_shared/seqtree/persistent_seq.hpp、値を葉に持つ重み平衡木) を、永続しない形で使ったもの。
struct Solver {
  PersistentSeq<MatMul, true, false> t;
  explicit Solver(const vector<Mat> &a) : t(a) {}
  void insert(int i, const Mat &m) { t.insert(i, m); }
  void erase(int i) { t.erase(i); }
  void reverse(int l, int r) { t.reverse(l, r); }
  Mat prod(int l, int r) { return t.prod(l, r); }
};
