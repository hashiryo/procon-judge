#pragma once
#include "_lib_composite.hpp"
#include "mylib/data_structure/SegmentTree_Dynamic.hpp"

// 今の Library の動的セグメント木。添字の空間を 2^30 に取り、set で触った葉とその祖先だけ節を new で作る。
struct Solver {
  SegmentTree_Dynamic<RcompositeQ, false, 30> seg;

  explicit Solver(int) {}

  void set(int p, i64 c, i64 d) { seg.set(p, {Mint(c), Mint(d)}); }

  i64 query(int l, int r, i64 x) {
    const auto t = seg.prod(l, r);
    return (t.first * Mint(x) + t.second).val();
  }
};
