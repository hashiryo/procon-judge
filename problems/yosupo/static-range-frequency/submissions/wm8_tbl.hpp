#pragma once
// 8 分の wavelet matrix (problems/_shared/wavelet/wm8.hpp) の count_eq。段の数が wm4 の 2/3 で、両端とも段ごとに popcount 1 回。
// x の順位は wm4_tbl と同じく上位 16 bit の表で狭めてから lower_cm で探す。
#include "pj.hpp"
#include "_shared/wavelet/wm8.hpp"
struct Solver {
 wavelet::Compressed c;
 wavelet::WM8 wm;
 wavelet::Table tbl;
 explicit Solver(const vector<int>& a): c(wavelet::compress(a)), wm(c.id, c.vals.size()), tbl(c.vals) {}
 int freq(int l, int r, int x) const {
  const int id= tbl.lower_cm(x);
  if(id == int(c.vals.size()) || c.vals[id] != x) return 0;
  return wm.count_eq(l, r, id);
 }
};
