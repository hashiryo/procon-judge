#pragma once
#include "common.hpp"
#include "mylib/data_structure/WeightBalancedTree.hpp"

// 重み平衡木。ノードを静的な配列に持つので、列の長さに関係なく LEAF_SIZE
// ぶんを最初に取る。メモリの記録が他より大きく出るのはそのため。
struct Solver {
  WeightBalancedTree<Raffine> wbt;

  explicit Solver(const vector<i64> &a) : wbt(to_mint(a)) {}

  void affine(int l, int r, i64 b, i64 c) { wbt.apply(l, r, {Mint(b), Mint(c)}); }

  i64 get(int i) { return wbt[i].val(); }
};
