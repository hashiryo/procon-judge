#pragma once
#include "common.hpp"
#include "_shared/seqtree/chunk_seq.hpp"

// 列の木の汎用の形 (problems/_shared/seqtree/chunk_seq.hpp、節が値の配列を持つ treap)。塊の大きさは 既定 (値が 16 byte なので 32)。
struct Solver {
  ChunkSeq<MatMul, true> t;
  explicit Solver(const vector<Mat> &a) : t(a) {}
  void insert(int i, const Mat &m) { t.insert(i, m); }
  void erase(int i) { t.erase(i); }
  void reverse(int l, int r) { t.reverse(l, r); }
  Mat prod(int l, int r) { return t.prod(l, r); }
};
