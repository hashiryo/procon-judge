#pragma once
#include "_lib_composite.hpp"
#include "mylib/data_structure/SegmentTree_Patricia.hpp"

// 今の Library のパトリシア木の上のセグメント木。枝分かれの無い道を 1 つの節に畳むので、節の数は
// set した添字の数の 2 倍に収まる。
struct Solver {
  SegmentTree_Patricia<RcompositeQ, false, 30> seg;

  explicit Solver(int) {}

  void set(int p, i64 c, i64 d) { seg.set(p, {Mint(c), Mint(d)}); }

  i64 query(int l, int r, i64 x) {
    const auto t = seg.prod(l, r);
    return (t.first * Mint(x) + t.second).val();
  }
};
