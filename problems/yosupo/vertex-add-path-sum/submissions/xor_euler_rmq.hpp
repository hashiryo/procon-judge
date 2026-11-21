#pragma once
#include "_euler.hpp"
#include "neo/base/ds/range/RangeMin.hpp"

// 根からの和を差分の BIT で持ち、u と v の道の和を S(u) + S(v) - 2 S(w) + a_w で求める (中身は _euler.hpp)。
// lca は、位置 i に親の位置を置いた列の区間最小で O(1) で求める (lca の rmq_lca と同じ考え方)。区間最小は
// NeoLibrary の RangeMin。木は隣の番号の xor で葉から剥がして作る。
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
struct Solver : EulerPathSum<RmqLca> {
  using EulerPathSum<RmqLca>::EulerPathSum;
};
