#pragma once
#include "common.hpp"
#include "mylib/data_structure/WeightBalancedTree.hpp"

// 重み平衡木。ノードを静的な配列から取るので、切り貼りで確保が起きない。
struct Solver {
  using Tree = WeightBalancedTree<int>;

  vector<Tree> ts;

  explicit Solver(int n) : ts(n) {}

  void push_back(int t, i64 x) { ts[t].push_back((int)x); }

  vector<i64> dump(int t) { return to_i64(ts[t].dump()); }

  void concat(int s, int t) { ts[t] += ts[s], ts[s].clear(); }
};
