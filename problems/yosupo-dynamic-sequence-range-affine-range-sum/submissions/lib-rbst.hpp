#pragma once
#include "common.hpp"
#include "mylib/data_structure/RandomizedBinarySearchTree.hpp"

// 乱択平衡二分探索木。merge と split で区間を切り出す。木の形が乱数で決まるので、操作列に左右されない。
struct Solver {
  using Tree = RandomizedBinarySearchTree<RaffineSum, true>;

  Tree t;

  explicit Solver(const vector<i64> &a) : t(to_mint(a)) {}

  void insert(int i, i64 x) { t.insert(i, Mint(x)); }

  void erase(int i) { t.erase(i); }

  void reverse(int l, int r) { t.reverse(l, r); }

  void affine(int l, int r, i64 b, i64 c) { t.apply(l, r, {Mint(b), Mint(c)}); }

  i64 sum(int l, int r) { return t.prod(l, r).val(); }
};
