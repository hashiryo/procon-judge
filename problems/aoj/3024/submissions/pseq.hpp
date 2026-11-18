#pragma once
#include "common.hpp"
#include "_shared/seqtree/persistent_seq.hpp"

// 永続の列の木の試作 (problems/_shared/seqtree/persistent_seq.hpp、値を葉に持つ重み平衡木)。列を丸ごと写す操作は根の写しで済み、
// 置き場が伸びたら 2 本の列から届く節だけを gc で詰める。
struct Solver {
  using P = PersistentSeq<RangeMin>;
  P t[2];
  Solver(const vector<i64> &a, const vector<i64> &b) : t{P(to_int(a)), P(to_int(b))} {}
  void set(int k, int i, i64 v) {
    t[k].set(i, (int)v);
    if (P::pool_empty()) P::gc(t[0], t[1]);
  }
  i64 min_of(int k, int l, int r) { return t[k].prod(l, r); }
  void assign(int dst, int src) { t[dst] = t[src]; }
};
