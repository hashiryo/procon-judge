#pragma once
#include "common.hpp"
#include "_shared/seqtree/wbt_seq.hpp"

// 今の Library の WeightBalancedTree の中身を写した列の木の試作 (problems/_shared/seqtree/wbt_seq.hpp) を、永続しない形
// (PERSIST = false) で使ったもの。split で外れた節と消した葉を使い回す。
struct Solver {
  WbtSeq<MatMul, true, false> t;
  explicit Solver(const vector<Mat> &a) : t(a) {}
  void insert(int i, const Mat &m) { t.insert(i, m); }
  void erase(int i) { t.erase(i); }
  void reverse(int l, int r) { t.reverse(l, r); }
  Mat prod(int l, int r) { return t.prod(l, r); }
};
