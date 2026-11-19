#pragma once
#include "common.hpp"
#include "_shared/seqtree/seq2.hpp"

// NeoLibrary の Sequence (塊の形) の手直しの試作 (problems/_shared/seqtree/seq2.hpp)。値を並べるだけの列で、n 本が同じ置き場を共有する。
struct Solver {
  using Tree = Seq2<int>;
  vector<Tree> ts;
  explicit Solver(int n) : ts(n) {}
  void push_back(int t, i64 x) { ts[t].push_back((int)x); }
  vector<i64> dump(int t) { return to_i64(ts[t].dump()); }
  void concat(int s, int t) { ts[t] += ts[s], ts[s].clear(); }
};
