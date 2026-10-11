#pragma once
// 今の Library の WaveletMatrix (mylib/data_structure/WaveletMatrix.hpp)。d 未満の個数 c を数え、c 番目と c - 1 番目に
// 小さい値を k 番目で引いて、d 以上の最小と d 未満の最大にする。
#include "pj.hpp"
#include "mylib/data_structure/WaveletMatrix.hpp"
struct Solver {
 WaveletMatrix<int> wm;
 explicit Solver(const vector<int>& a): wm(a) {}
 int closest(int l, int r, int d) const {
  const int c= wm.count(l, r, d);
  if(c == 0) return wm.kth_smallest(l, r, 0) - d;
  if(c == r - l) return d - wm.kth_largest(l, r, 0);
  return min(wm.kth_smallest(l, r, c) - d, d - wm.kth_smallest(l, r, c - 1));
 }
};
