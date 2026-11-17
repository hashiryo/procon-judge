#pragma once
#include "pj.hpp"
#include "_shared/pages/stagger_new.hpp"
#include "neo/data_structure/segtree/AddSum.hpp"

// 確かめ用の写しで、lib-neo と同じ中身に、2 MB 境界で取る配列の先頭を取るたびに 2 KB と 64 byte ずつずらす差し替え (_shared/pages/stagger_new.hpp) を足したもの。
// NeoLibrary の SegmentTree に monoid::AddSum<long long> を渡す。中身は差分を 2 本の 8 分木の接頭辞和に持つ形。
struct Solver {
  SegmentTree<monoid::AddSum<i64>> seg;
  explicit Solver(const vector<i64> &a) : seg(a) {}
  void add(int l, int r, i64 x) { seg.apply(l, r, x); }
  i64 sum(int l, int r) { return seg.prod(l, r); }
};
