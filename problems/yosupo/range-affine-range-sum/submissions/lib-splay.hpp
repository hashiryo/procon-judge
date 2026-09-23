#pragma once
#include "common.hpp"
#include "mylib/data_structure/SplayTree.hpp"

// スプレー木。触った位置を根へ回転で持ち上げるので、木の形が操作列に依存する。
// 償却の計算量なので、1 クエリの最悪は他より悪い。
struct Solver {
  SplayTree<RaffineSum> st;

  explicit Solver(const vector<i64> &a) : st(to_mint(a)) {}

  void affine(int l, int r, i64 b, i64 c) { st.apply(l, r, {Mint(b), Mint(c)}); }

  i64 sum(int l, int r) { return st.prod(l, r).val(); }
};
