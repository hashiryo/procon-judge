#pragma once
#include "common.hpp"
#include "mylib/data_structure/WeightBalancedTree.hpp"

// 重み平衡木。ノードを静的な配列から取るので、挿入と削除で確保が起きない。列の長さに関係なく LEAF_SIZE ぶんを最初に取る。
struct Solver {
  using Tree = WeightBalancedTree<RaffineSum, true>;

  Tree t;

  explicit Solver(const vector<i64> &a) : t(to_mint(a)) {}

  void insert(int i, i64 x) { t.insert(i, Mint(x)); }

  void erase(int i) { t.erase(i); }

  void reverse(int l, int r) { t.reverse(l, r); }

  void affine(int l, int r, i64 b, i64 c) { t.apply(l, r, {Mint(b), Mint(c)}); }

  i64 sum(int l, int r) { return t.prod(l, r).val(); }
};
