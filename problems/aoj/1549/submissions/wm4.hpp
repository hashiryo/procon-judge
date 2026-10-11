#pragma once
// 4 分の wavelet matrix (problems/_shared/wavelet/wm4.hpp)。値は負も受ける基数ソートで順位に詰め、d の順位は値の範囲に合わせた表 (Lookup) で引く。d 未満の個数 c を数え、
// k 番目で c - 1 番目 (d 未満の最大) と c 番目 (d 以上の最小) を引く。
#include "pj.hpp"
#include "_shared/wavelet/wm4.hpp"
struct Solver {
 wavelet::Compressed c;
 wavelet::WM4<> wm;
 wavelet::Lookup lk;
 explicit Solver(const vector<int>& a): c(wavelet::compress_s(a)), wm(c.id, c.vals.size()), lk(c.vals) {}
 int closest(int l, int r, int d) const {
  const int k= wm.count_less(l, r, lk.lower(d));
  int best= INT32_MAX;
  if(k > 0) best= d - c.vals[wm.kth(l, r, k - 1)];
  if(k < r - l) best= min(best, c.vals[wm.kth(l, r, k)] - d);
  return best;
 }
};
