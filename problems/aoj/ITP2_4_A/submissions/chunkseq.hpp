#pragma once
#include "common.hpp"
#include "_shared/seqtree/chunk_seq.hpp"

// 列の木の汎用の形 (problems/_shared/seqtree/chunk_seq.hpp、節が値の配列を持つ treap)。
struct Solver {
  ChunkSeq<int, true> t;
  explicit Solver(const vector<i64> &a) : t(to_int(a)) {}
  void reverse(int b, int e) { t.reverse(b, e); }
  vector<i64> dump() { return to_i64(t.dump()); }
};
