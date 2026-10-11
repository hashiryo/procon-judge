#pragma once
// wm_sep_br (重みの累積を y の境の bit が 1 の段だけ読む) と、x の範囲と y の境の順位を上位 16 bit の表で狭めてから
// lower_cm で探す形を合わせたもの。
#include "pj.hpp"
#include "_shared/wavelet/wm.hpp"
#include "_points.hpp"
struct Solver {
 Points p;
 wavelet::WaveletMatrix<wavelet::Sep, true> wm;
 wavelet::Table tx, ty;
 explicit Solver(const vector<array<int, 3>>& pts): p(pts), wm(p.yid, p.ys.size(), &p.w), tx(p.xs), ty(p.ys) {}
 i64 rect_sum(int l, int d, int r, int u) const {
  const int i= tx.lower_cm(l), j= tx.lower_cm(r);
  return wm.count_sum_less_br(i, j, ty.lower_cm(u)).second - wm.count_sum_less_br(i, j, ty.lower_cm(d)).second;
 }
};
