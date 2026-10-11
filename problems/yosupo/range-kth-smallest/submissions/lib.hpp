#pragma once
// 今の Library の WaveletMatrix (mylib/data_structure/WaveletMatrix.hpp)。値を順位に詰め、段ごとに 32 bit の語の
// vector と累積の vector を持つ。段の数は列の長さの bit 数。
#include "pj.hpp"
#include "mylib/data_structure/WaveletMatrix.hpp"
struct Solver {
 WaveletMatrix<int> wm;
 explicit Solver(const vector<int>& a): wm(a) {}
 int kth_smallest(int l, int r, int k) const { return wm.kth_smallest(l, r, k); }
};
