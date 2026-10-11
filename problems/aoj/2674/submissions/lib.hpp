#pragma once
// 今の Library の WaveletMatrix (mylib/data_structure/WaveletMatrix.hpp) の count(l, r, lo, hi)。
#include "pj.hpp"
#include "mylib/data_structure/WaveletMatrix.hpp"
struct Solver {
 WaveletMatrix<int> wm;
 explicit Solver(const vector<int>& x): wm(x) {}
 int count(int l, int r, int lo, int hi) const { return wm.count(l, r, lo, hi); }
};
