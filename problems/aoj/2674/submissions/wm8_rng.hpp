#pragma once
// wm8 の [lo, hi) の個数を、2 回下りる形でなく、2 つの境の記号が同じ段を 1 回で下り、違う段から下を 2 本同じループで下りる形
// (problems/_shared/wavelet/wm8.hpp の count_range) にしたもの。
#include "pj.hpp"
#include "_shared/wavelet/wm8.hpp"
struct Solver {
 wavelet::Compressed c;
 wavelet::WM8 wm;
 wavelet::Lookup lk;
 explicit Solver(const vector<int>& x): c(wavelet::compress_s(x)), wm(c.id, c.vals.size()), lk(c.vals) {}
 int count(int l, int r, int lo, int hi) const {
  const int a= lk.lower(lo), b= lk.lower(hi);
  return a < b ? wm.count_range(l, r, a, b) : 0;
 }
};
