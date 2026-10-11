#pragma once
// wm_sep の x の範囲と y の境の順位を、上位 16 bit の表 (problems/_shared/wavelet/wm.hpp の Table) で区間に狭めてから
// 探す形。x の表は重複のある並べた x の上に作る。
#include "pj.hpp"
#include "_shared/wavelet/wm.hpp"
#include "_points.hpp"
struct Solver {
 Points p;
 wavelet::WaveletMatrix<wavelet::Sep, true> wm;
 wavelet::Table tx, ty;
 explicit Solver(const vector<array<int, 3>>& pts): p(pts), wm(p.yid, p.ys.size(), &p.w), tx(p.xs), ty(p.ys) {}
 i64 rect_sum(int l, int d, int r, int u) const {
  const int i= tx.lower(l), j= tx.lower(r);
  return wm.count_sum_less(i, j, ty.lower(u)).second - wm.count_sum_less(i, j, ty.lower(d)).second;
 }
};
