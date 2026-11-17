#pragma once
#include "_wide_sum.hpp"
#include "neo/data_structure/RangeMin.hpp"

// xor_euler_rmq の BIT を、節を 8 個に広げた接頭辞和の木に替えたもの (_wide_sum.hpp)。根からの和の差分を持ち、
// u と v の道の和を S(u) + S(v) - 2 S(w) + a_w で求める。lca は親の位置の列の区間最小 (NeoLibrary の RangeMin) で O(1)。
struct RmqLca {
  RangeMin rm;
  static vector<u32> up(const path_hld::Tree &t) { return vector<u32>(t.upp, t.upp + t.n); }
  explicit RmqLca(const path_hld::Tree &t) : rm(up(t)) {}
  int operator()(int x, int y) const {
    if (x == y) return x;
    if (x > y) swap(x, y);
    return int(rm.min(x + 1, y + 1));
  }
};
struct Solver : EulerPathSum<RmqLca, WideSum> {
  using EulerPathSum<RmqLca, WideSum>::EulerPathSum;
};
