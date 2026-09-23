#pragma once
#include "common.hpp"
#include "mylib/data_structure/RandomizedBinarySearchTree.hpp"

// 乱択平衡二分探索木。区間の操作は split して merge で戻す形になる。
// この問題は列の長さが変わらないので、平衡木の柔軟さは使っていない。
struct Solver {
  RandomizedBinarySearchTree<RaffineSum> rbst;

  explicit Solver(const vector<i64> &a) : rbst(to_mint(a)) {}

  void affine(int l, int r, i64 b, i64 c) {
    rbst.apply(l, r, {Mint(b), Mint(c)});
  }

  i64 sum(int l, int r) { return rbst.prod(l, r).val(); }
};
