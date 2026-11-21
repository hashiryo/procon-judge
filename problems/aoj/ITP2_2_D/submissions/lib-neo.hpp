#pragma once
#include "common.hpp"
#include "neo/base/ds/seq/Sequence.hpp"

// NeoLibrary の neo/base/ds/seq/Sequence.hpp (塊の形)。値を並べるだけの列で、n 本が同じ置き場を共有する。
struct Solver {
  using Tree = Sequence<int>;
  vector<Tree> ts;
  explicit Solver(int n) : ts(n) {}
  void push_back(int t, i64 x) { ts[t].push_back((int)x); }
  vector<i64> dump(int t) { return to_i64(ts[t].dump()); }
  void concat(int s, int t) { ts[t] += ts[s], ts[s].clear(); }
};
