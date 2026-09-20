#pragma once
#include "common.hpp"
#include "mylib/data_structure/RandomizedBinarySearchTree.hpp"

// 乱択平衡二分探索木。木の形が乱数で決まるので、操作列に左右されない。
struct Solver {
  using Tree = RandomizedBinarySearchTree<RangeMin>;

  Tree t;

  explicit Solver(const vector<i64> &a) : t(to_int(a)) {}

  // 末尾を抜いて先頭に挿す。切り出した区間の中だけで完結する。
  void rotate(int l, int r) {
    auto [a, b, c] = t.split3(l, r);
    b.push_front(b.pop_back());
    t = a + b + c;
  }

  i64 min_of(int l, int r) { return t.prod(l, r); }

  void set(int i, i64 x) { t.set(i, (int)x); }
};
