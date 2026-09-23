#pragma once
#include "common.hpp"
#include "mylib/data_structure/SegmentTree.hpp"

// 遅延伝搬つきのセグメント木。ノードを配列で持つ非再帰の実装で、この問題では
// いちばん素直な形。他の 4 本は平衡二分探索木なので、その基準線になる。
struct Solver {
  SegmentTree<RaffineSum> seg;

  explicit Solver(const vector<i64> &a) : seg(to_mint(a)) {}

  void affine(int l, int r, i64 b, i64 c) { seg.apply(l, r, {Mint(b), Mint(c)}); }

  i64 sum(int l, int r) { return seg.prod(l, r).val(); }
};
