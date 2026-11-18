#pragma once
#include "common.hpp"
#include "_shared/seqtree/chunk_seq.hpp"

// 列の木の汎用の形 (problems/_shared/seqtree/chunk_seq.hpp、節が値の配列を持つ treap)。区間を回すのは、末尾を消して先頭に入れる
// (切り出さない)。
struct Solver {
  struct M {
    using T = int;
    static T ti() { return 1 << 30; }
    static T op(T l, T r) { return l < r ? l : r; }
    using commute = void;
  };
  ChunkSeq<M> t;
  explicit Solver(const vector<i64> &a) : t(to_int(a)) {}
  void rotate(int l, int r) { t.insert(l, t.erase(r - 1)); }
  i64 min_of(int l, int r) { return t.prod(l, r); }
  void set(int i, i64 x) { t.set(i, (int)x); }
};
