#pragma once
// 4 分の wavelet matrix (problems/_shared/wavelet/wm4.hpp) の count_sum_less を、y の順位の列と重みの上に作る。段ごとに
// 記号が 1 未満、2 未満、3 未満の重みの累積を 3 本持つ。x の範囲と y の境の順位は、上位 16 bit の表で狭めてから
// lower_cm で探す。
#include "pj.hpp"
#include "_shared/wavelet/wm4.hpp"
#include "_points.hpp"
struct Solver {
 Points p;
 wavelet::WM4<true> wm;
 wavelet::Table tx, ty;
 explicit Solver(const vector<array<int, 3>>& pts): p(pts), wm(p.yid, p.ys.size(), &p.w), tx(p.xs), ty(p.ys) {}
 i64 rect_sum(int l, int d, int r, int u) const {
  const int i= tx.lower_cm(l), j= tx.lower_cm(r);
  return wm.count_sum_less(i, j, ty.lower_cm(u)).second - wm.count_sum_less(i, j, ty.lower_cm(d)).second;
 }
};
