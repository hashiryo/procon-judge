#pragma once
#include "common.hpp"
#include "mylib/data_structure/SegmentTree_Dynamic.hpp"

// 動的セグメント木。添字の空間を 2 の冪で取っておいて、ノードは触ったところ
// だけ作る。この問題は全区間が埋まるので作る数は静的なものと変わらず、
// ポインタで辿るぶんの差が出る。
struct Solver {
  SegmentTree_Dynamic<RaffineSum> seg;

  explicit Solver(const vector<i64> &a) : seg(to_mint(a)) {}

  void affine(int l, int r, i64 b, i64 c) { seg.apply(l, r, {Mint(b), Mint(c)}); }

  i64 sum(int l, int r) { return seg.prod(l, r).val(); }
};
