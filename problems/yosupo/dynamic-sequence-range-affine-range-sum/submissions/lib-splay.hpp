#pragma once
#include "common.hpp"
#include "mylib/data_structure/SplayTree.hpp"

// スプレー木。触った位置を根へ回転で持ち上げるので、木の形が操作列に依存する。償却の計算量になる。
struct Solver {
  using Tree = SplayTree<RaffineSum, true>;

  Tree t;

  explicit Solver(const vector<i64> &a) : t(to_mint(a)) {}

  void insert(int i, i64 x) { t.insert(i, Mint(x)); }

  void erase(int i) { t.erase(i); }

  void reverse(int l, int r) { t.reverse(l, r); }

  void affine(int l, int r, i64 b, i64 c) { t.apply(l, r, {Mint(b), Mint(c)}); }

  i64 sum(int l, int r) { return t.prod(l, r).val(); }
};
