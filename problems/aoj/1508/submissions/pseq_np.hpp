#pragma once
#include "common.hpp"
#include "_shared/seqtree/persistent_seq.hpp"

// 永続の列の木の試作 (problems/_shared/seqtree/persistent_seq.hpp、値を葉に持つ重み平衡木) を、永続しない形で使ったもの。区間を
// 回すのは chunkseq と同じく、末尾を消して先頭に入れる。
struct Solver {
  struct M {
    using T = int;
    static T ti() { return 1 << 30; }
    static T op(T l, T r) { return l < r ? l : r; }
    using commute = void;
  };
  PersistentSeq<M, false, false> t;
  explicit Solver(const vector<i64> &a) : t(to_int(a)) {}
  void rotate(int l, int r) { t.insert(l, t.erase(r - 1)); }
  i64 min_of(int l, int r) { return t.prod(l, r); }
  void set(int i, i64 x) { t.set(i, (int)x); }
};
