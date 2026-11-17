#pragma once
#include "_wide_sum.hpp"

// xor_euler_hld の BIT を、節を 8 個に広げた接頭辞和の木に替えたもの (_wide_sum.hpp)。根からの和の差分を持ち、
// u と v の道の和を S(u) + S(v) - 2 S(w) + a_w で求める。lca は HLD の鎖をたどって求める。
struct HldLca {
  const path_hld::Tree *t;
  explicit HldLca(const path_hld::Tree &t) : t(&t) {}
  int operator()(int x, int y) const { return t->lca_pos(x, y); }
};
struct Solver : EulerPathSum<HldLca, WideSum> {
  using EulerPathSum<HldLca, WideSum>::EulerPathSum;
};
