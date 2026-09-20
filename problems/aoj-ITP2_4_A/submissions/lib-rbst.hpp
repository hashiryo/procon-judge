#pragma once
#include "common.hpp"
#include "mylib/data_structure/RandomizedBinarySearchTree.hpp"

// 乱択平衡二分探索木。木の形が乱数で決まるので、操作列に左右されない。
struct Solver {
  using Tree = RandomizedBinarySearchTree<int, true>;

  Tree t;

  explicit Solver(const vector<i64> &a) : t(to_int(a)) {}

  void reverse(int b, int e) { t.reverse(b, e); }

  vector<i64> dump() { return to_i64(t.dump()); }
};
