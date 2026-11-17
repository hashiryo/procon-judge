#pragma once
#include "_euler.hpp"

// 根からの和を差分の BIT で持ち、u と v の道の和を S(u) + S(v) - 2 S(w) + a_w で求める (中身は _euler.hpp)。
// lca は HLD の鎖をたどって求める。木は隣の番号の xor で葉から剥がして作る。
struct HldLca {
  const path_hld::Tree *t;
  explicit HldLca(const path_hld::Tree &t) : t(&t) {}
  int operator()(int x, int y) const { return t->lca_pos(x, y); }
};
struct Solver : EulerPathSum<HldLca> {
  using EulerPathSum<HldLca>::EulerPathSum;
};
