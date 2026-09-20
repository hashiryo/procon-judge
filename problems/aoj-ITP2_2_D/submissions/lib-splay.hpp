#pragma once
#include "common.hpp"
#include "mylib/data_structure/SplayTree.hpp"

// スプレー木。触った位置を根へ回転で持ち上げるので、木の形が操作列に依存する。
struct Solver {
  using Tree = SplayTree<int>;

  vector<Tree> ts;

  explicit Solver(int n) : ts(n) {}

  void push_back(int t, i64 x) { ts[t].push_back((int)x); }

  vector<i64> dump(int t) { return to_i64(ts[t].dump()); }

  void concat(int s, int t) { ts[t] += ts[s], ts[s].clear(); }
};
