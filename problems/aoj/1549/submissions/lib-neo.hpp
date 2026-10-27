#pragma once
// NeoLibrary の WaveletMatrix (neo/data_structure/WaveletMatrix.hpp) の max_le と min_ge。この問題で書き比べた wm4 を写したもの。
#include "pj.hpp"
#include "neo/data_structure/WaveletMatrix.hpp"
struct Solver {
 WaveletMatrix<int> wm;
 explicit Solver(const vector<int>& a): wm(a) {}
 int closest(int l, int r, int d) const {
  int best= INT32_MAX;
  if(auto p= wm.max_le(l, r, d)) best= d - *p;
  if(auto q= wm.min_ge(l, r, d)) best= min(best, *q - d);
  return best;
 }
};
