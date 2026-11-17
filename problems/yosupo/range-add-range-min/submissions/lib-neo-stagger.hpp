#pragma once
#include "pj.hpp"
#include "_shared/pages/stagger_new.hpp"
#include "neo/data_structure/segtree/AddMin.hpp"

// 確かめ用の写しで、lib-neo と同じ中身に、2 MB 境界で取る配列の先頭を取るたびに 2 KB と 64 byte ずつずらす差し替え (_shared/pages/stagger_new.hpp) を足したもの。
// NeoLibrary の SegmentTree に monoid::AddMin<long long> を渡す。中身は作用を配らない 16 分木 (wide3 と同じ)。
struct Solver {
  SegmentTree<monoid::AddMin<i64>> seg;
  explicit Solver(const vector<i64> &a) : seg(a) {}
  void add(int l, int r, i64 x) { seg.apply(l, r, x); }
  i64 minimum(int l, int r) { return seg.prod(l, r); }
};
