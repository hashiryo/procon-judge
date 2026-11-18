#pragma once
#include "common.hpp"
#include "_shared/seqtree/wbt_seq.hpp"

// 今の Library の WeightBalancedTree の中身を写した列の木の試作 (problems/_shared/seqtree/wbt_seq.hpp) を、永続しない形
// (PERSIST = false) で使ったもの。split で外れた節と消した葉を使い回す。
struct Solver {
  struct M {
    using T = int;
    static T ti() { return 1 << 30; }
    static T op(T l, T r) { return l < r ? l : r; }
    using commute = void;
  };
  WbtSeq<M, false, false> t;
  explicit Solver(const vector<i64> &a) : t(to_int(a)) {}
  void rotate(int l, int r) { t.insert(l, t.erase(r - 1)); }
  i64 min_of(int l, int r) { return t.prod(l, r); }
  void set(int i, i64 x) { t.set(i, (int)x); }
};
