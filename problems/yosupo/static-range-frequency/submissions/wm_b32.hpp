#pragma once
// wavelet matrix (problems/_shared/wavelet/wm.hpp) の B32 の形。段ごとに、32 bit の bit とその前までの累積を 1 つの 64 bit の語に詰める。
// x の順位を二分探索で求め、無ければ 0、あれば x の bit をたどって下りた区間の長さを返す。
#include "pj.hpp"
#include "_shared/wavelet/wm.hpp"
struct Solver {
 wavelet::Compressed c;
 wavelet::WaveletMatrix<wavelet::B32> wm;
 explicit Solver(const vector<int>& a): c(wavelet::compress(a)), wm(c.id, c.vals.size()) {}
 int freq(int l, int r, int x) const {
  const int id= wavelet::lower(c.vals, x);
  if(id == int(c.vals.size()) || c.vals[id] != x) return 0;
  return wm.count_eq(l, r, id);
 }
};
