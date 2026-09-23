#pragma once
#include "common.hpp"
#include "mylib/data_structure/RandomizedBinarySearchTree.hpp"

// 乱択平衡二分探索木。木の形が乱数で決まるので、操作列に左右されない。
struct Solver {
  using Tree = RandomizedBinarySearchTree<int>;

  vector<Tree> ts;

  explicit Solver(int n) : ts(n) {}

  void push_back(int t, i64 x) { ts[t].push_back((int)x); }

  vector<i64> dump(int t) { return to_i64(ts[t].dump()); }

  void concat(int s, int t) { ts[t] += ts[s], ts[s].clear(); }
};
