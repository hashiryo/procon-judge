#pragma once
// wm8 の k 番目を、区間の中の数を配列に書かずに比べる形 (problems/_shared/wavelet/wm8.hpp の kthp) にしたもの。wm8 の kth は
// 8 個の 32 bit の書き込みを 1 回のベクタで読み直していた。ほかは wm8 と同じ。
#include "pj.hpp"
#include "_shared/wavelet/wm8.hpp"
struct Solver {
 wavelet::Compressed c;
 wavelet::WM8 wm;
 wavelet::Lookup lk;
 explicit Solver(const vector<int>& a): c(wavelet::compress_s(a)), wm(c.id, c.vals.size()), lk(c.vals) {}
 int closest(int l, int r, int d) const {
  const int k= wm.count_less(l, r, lk.lower(d));
  int best= INT32_MAX;
  if(k > 0) best= d - c.vals[wm.kthp(l, r, k - 1)];
  if(k < r - l) best= min(best, c.vals[wm.kthp(l, r, k)] - d);
  return best;
 }
};
