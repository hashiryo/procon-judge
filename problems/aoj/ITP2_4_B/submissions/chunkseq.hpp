#pragma once
#include "common.hpp"
#include "_shared/seqtree/chunk_seq.hpp"

// 列の木の汎用の形 (problems/_shared/seqtree/chunk_seq.hpp、節が値の配列を持つ treap)。
struct Solver {
  ChunkSeq<int> t;
  explicit Solver(const vector<i64> &a) : t(to_int(a)) {}
  // 3 つに割って、真ん中をさらに 2 つに割り、順を入れ替えて繋ぎ直す。
  void rotate(int b, int m, int e) {
    auto [l, c, r] = t.split3(b, e);
    auto [cl, cr] = c.split(m - b);
    t = l + cr + cl + r;
  }
  vector<i64> dump() { return to_i64(t.dump()); }
};
