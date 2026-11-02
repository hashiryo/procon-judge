#pragma once
// NeoLibrary の WaveletMatrix (neo/data_structure/WaveletMatrix.hpp)。d 未満の個数 c を count_lt で 1 回数え、kth_smallest で
// c - 1 番目 (d 未満の最大) と c 番目 (d 以上の最小) を引く (wm4 と同じ形)。max_le と min_ge で引いた形は、個数を 2 回数えるので
// x64-gcc で wm4 より 1 割 5 分ほど遅かった (run 40477233432)。
#include "pj.hpp"
#include "neo/data_structure/WaveletMatrix.hpp"
struct Solver {
 WaveletMatrix<int> wm;
 explicit Solver(const vector<int>& a): wm(a) {}
 int closest(int l, int r, int d) const {
  const int c= wm.count_lt(l, r, d);
  int best= INT32_MAX;
  if(c > 0) best= d - wm.kth_smallest(l, r, c - 1);
  if(c < r - l) best= min(best, wm.kth_smallest(l, r, c) - d);
  return best;
 }
};
