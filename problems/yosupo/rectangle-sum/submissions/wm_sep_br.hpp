#pragma once
// wm_sep と同じ形で、段ごとの重みの累積を y の境の bit が 1 の段だけ読む (分岐で飛ばす) count_sum_less_br を使う。
#include "pj.hpp"
#include "_shared/wavelet/wm.hpp"
#include "_points.hpp"
struct Solver {
 Points p;
 wavelet::WaveletMatrix<wavelet::Sep, true> wm;
 explicit Solver(const vector<array<int, 3>>& pts): p(pts), wm(p.yid, p.ys.size(), &p.w) {}
 i64 rect_sum(int l, int d, int r, int u) const {
  const int i= wavelet::lower(p.xs, l), j= wavelet::lower(p.xs, r);
  return wm.count_sum_less_br(i, j, wavelet::lower(p.ys, u)).second - wm.count_sum_less_br(i, j, wavelet::lower(p.ys, d)).second;
 }
};
