#pragma once
#include "common.hpp"
#include "mylib/data_structure/WeightBalancedTree.hpp"

// 重み平衡木。ノードを静的な配列から取るので、切り貼りで確保が起きない。
struct Solver {
  using Tree = WeightBalancedTree<int>;

  Tree t;

  explicit Solver(const vector<i64> &a) : t(to_int(a)) {}

  // 3 つに割って、真ん中をさらに 2 つに割り、順を入れ替えて繋ぎ直す。
  void rotate(int b, int m, int e) {
    auto [l, c, r] = t.split3(b, e);
    auto [cl, cr] = c.split(m - b);
    t = l + cr + cl + r;
  }

  vector<i64> dump() { return to_i64(t.dump()); }
};
