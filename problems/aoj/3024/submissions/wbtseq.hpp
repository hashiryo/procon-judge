#pragma once
#include "common.hpp"
#include "_shared/seqtree/wbt_seq.hpp"

// 今の Library の WeightBalancedTree の中身を写し、伸びる置き場と gc を足した列の木の試作 (problems/_shared/seqtree/wbt_seq.hpp)
// で書いたもの。置き場が伸びたら gc で生きている版から届く節だけを詰める。
struct Solver {
  using P = WbtSeq<RangeMin>;
  P t[2];
  Solver(const vector<i64> &a, const vector<i64> &b) : t{P(to_int(a)), P(to_int(b))} {}
  void set(int k, int i, i64 v) {
    t[k].set(i, (int)v);
    if (P::pool_empty()) P::gc(t[0], t[1]);
  }
  i64 min_of(int k, int l, int r) { return t[k].prod(l, r); }
  void assign(int dst, int src) { t[dst] = t[src]; }
};
