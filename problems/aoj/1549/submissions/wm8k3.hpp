#pragma once
// wm8 の k 番目を、段の中の 3 bit を上から 1 bit ずつ選ぶ形 (problems/_shared/wavelet/wm8.hpp の kth3) にしたもの。段ごとの
// popcount は 6 回で、スタックの配列を使わない。ほかは wm8 と同じ。
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
  if(k > 0) best= d - c.vals[wm.kth3(l, r, k - 1)];
  if(k < r - l) best= min(best, c.vals[wm.kth3(l, r, k)] - d);
  return best;
 }
};
