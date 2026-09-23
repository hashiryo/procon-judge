#pragma once
#include "common.hpp"
#include "mylib/data_structure/SegmentTree.hpp"

// 遅延伝搬つきのセグメント木。双対だけを載せた形。配列で持つので、平衡二分
// 探索木の 3 本に対する基準線になる。
struct Solver {
  SegmentTree<Raffine> seg;

  explicit Solver(const vector<i64> &a) : seg(to_mint(a)) {}

  void affine(int l, int r, i64 b, i64 c) { seg.apply(l, r, {Mint(b), Mint(c)}); }

  i64 get(int i) { return seg[i].val(); }
};
