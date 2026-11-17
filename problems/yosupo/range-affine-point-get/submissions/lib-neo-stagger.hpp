#pragma once
#include "pj.hpp"
#include "_shared/pages/stagger_new.hpp"
#include "neo/data_structure/segtree/Affine.hpp"

// 確かめ用の写しで、lib-neo と同じ中身に、2 MB 境界で取る配列の先頭を取るたびに 2 KB と 64 byte ずつずらす差し替え (_shared/pages/stagger_new.hpp) を足したもの。
// NeoLibrary の SegmentTree に monoid::Affine<ZMod<998244353>> を渡す。中身は 16 分木の双対セグ木 (wide_dual2_pf と同じ)。
struct Solver {
  using Z = ZMod<998244353>;
  SegmentTree<monoid::Affine<Z>> seg;
  explicit Solver(const vector<i64> &a) : seg(int(a.size()), [&](int i) { return Z(a[i]); }) {}
  void affine(int l, int r, i64 b, i64 c) { seg.apply(l, r, {Z(b), Z(c)}); }
  i64 get(int i) { return seg.get(i).val(); }
};
