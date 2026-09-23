#pragma once
#include "common.hpp"
#include "mylib/data_structure/SplayTree.hpp"

// スプレー木。触った位置を根へ回転で持ち上げるので、木の形が操作列に依存する。
struct Solver {
  SplayTree<Raffine> st;

  explicit Solver(const vector<i64> &a) : st(to_mint(a)) {}

  void affine(int l, int r, i64 b, i64 c) { st.apply(l, r, {Mint(b), Mint(c)}); }

  i64 get(int i) { return st[i].val(); }
};
