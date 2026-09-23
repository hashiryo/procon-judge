#pragma once
#include "common.hpp"
#include "mylib/data_structure/RandomizedBinarySearchTree.hpp"

// 乱択平衡二分探索木。区間の操作は split して merge で戻す形になる。
struct Solver {
  RandomizedBinarySearchTree<Raffine> rbst;

  explicit Solver(const vector<i64> &a) : rbst(to_mint(a)) {}

  void affine(int l, int r, i64 b, i64 c) {
    rbst.apply(l, r, {Mint(b), Mint(c)});
  }

  i64 get(int i) { return rbst[i].val(); }
};
