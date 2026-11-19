#pragma once
#include "common.hpp"
#include "_shared/seqtree/seq2.hpp"

// NeoLibrary の Sequence (塊の形) の手直しの試作 (problems/_shared/seqtree/seq2.hpp)。区間を回すのは、末尾を消して先頭に入れる。
struct Solver {
  struct M {
    using T = int;
    static T ti() { return 1 << 30; }
    static T op(T l, T r) { return l < r ? l : r; }
    using commute = void;
  };
  Seq2<M> t;
  explicit Solver(const vector<i64> &a) : t(to_int(a)) {}
  void rotate(int l, int r) { t.insert(l, t.erase(r - 1)); }
  i64 min_of(int l, int r) { return t.prod(l, r); }
  void set(int i, i64 x) { t.set(i, (int)x); }
};
