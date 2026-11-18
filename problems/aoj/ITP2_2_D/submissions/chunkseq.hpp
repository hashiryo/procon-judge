#pragma once
#include "common.hpp"
#include "_shared/seqtree/chunk_seq.hpp"

// 列の木の汎用の形 (problems/_shared/seqtree/chunk_seq.hpp、節が値の配列を持つ treap)。値を並べるだけの列で、n 本が同じ置き場を
// 共有する。
struct Solver {
  using Tree = ChunkSeq<int>;
  vector<Tree> ts;
  explicit Solver(int n) : ts(n) {}
  void push_back(int t, i64 x) { ts[t].push_back((int)x); }
  vector<i64> dump(int t) { return to_i64(ts[t].dump()); }
  void concat(int s, int t) { ts[t] += ts[s], ts[s].clear(); }
};
