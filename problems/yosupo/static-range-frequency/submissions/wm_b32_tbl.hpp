#pragma once
// wm_b32 の x の順位を、上位 16 bit の表 (problems/_shared/wavelet/wm.hpp の Table) で区間に狭めてから探す形。
#include "pj.hpp"
#include "_shared/wavelet/wm.hpp"
struct Solver {
 wavelet::Compressed c;
 wavelet::WaveletMatrix<wavelet::B32> wm;
 wavelet::Table tbl;
 explicit Solver(const vector<int>& a): c(wavelet::compress(a)), wm(c.id, c.vals.size()), tbl(c.vals) {}
 int freq(int l, int r, int x) const {
  const int id= tbl.lower(x);
  if(id == int(c.vals.size()) || c.vals[id] != x) return 0;
  return wm.count_eq(l, r, id);
 }
};
