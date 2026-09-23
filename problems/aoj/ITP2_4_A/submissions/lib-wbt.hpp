#pragma once
#include "common.hpp"
#include "mylib/data_structure/WeightBalancedTree.hpp"

// 重み平衡木。ノードを静的な配列から取るので、切り貼りで確保が起きない。
struct Solver {
  using Tree = WeightBalancedTree<int, true>;

  Tree t;

  explicit Solver(const vector<i64> &a) : t(to_int(a)) {}

  void reverse(int b, int e) { t.reverse(b, e); }

  vector<i64> dump() { return to_i64(t.dump()); }
};
