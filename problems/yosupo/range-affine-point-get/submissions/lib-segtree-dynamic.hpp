#pragma once
#include "common.hpp"
#include "mylib/data_structure/SegmentTree_Dynamic.hpp"

// 動的セグメント木。この問題は全区間が埋まるのでノードの数は静的なものと
// 変わらず、ポインタで辿るぶんの差が出る。
struct Solver {
  SegmentTree_Dynamic<Raffine> seg;

  explicit Solver(const vector<i64> &a) : seg(to_mint(a)) {}

  void affine(int l, int r, i64 b, i64 c) { seg.apply(l, r, {Mint(b), Mint(c)}); }

  i64 get(int i) { return seg[i].val(); }
};
