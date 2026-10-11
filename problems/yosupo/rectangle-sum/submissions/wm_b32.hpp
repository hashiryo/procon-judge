#pragma once
// wavelet matrix (problems/_shared/wavelet/wm.hpp) の B32 の形。段ごとに、32 bit の bit とその前までの累積を 1 つの 64 bit の語に詰める。
// 点を x の順に並べ、y の順位の列の上に、段ごとの重みの累積を持つ wavelet matrix を作る。x の範囲は並べた x の
// 二分探索で位置の範囲に直し、y の範囲は u 未満の和から d 未満の和を引く (2 回下りる)。
#include "pj.hpp"
#include "_shared/wavelet/wm.hpp"
#include "_points.hpp"
struct Solver {
 Points p;
 wavelet::WaveletMatrix<wavelet::B32, true> wm;
 explicit Solver(const vector<array<int, 3>>& pts): p(pts), wm(p.yid, p.ys.size(), &p.w) {}
 i64 rect_sum(int l, int d, int r, int u) const {
  const int i= wavelet::lower(p.xs, l), j= wavelet::lower(p.xs, r);
  return wm.count_sum_less(i, j, wavelet::lower(p.ys, u)).second - wm.count_sum_less(i, j, wavelet::lower(p.ys, d)).second;
 }
};
