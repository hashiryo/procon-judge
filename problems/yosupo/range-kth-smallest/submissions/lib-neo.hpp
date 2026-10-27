#pragma once
// NeoLibrary の WaveletMatrix (neo/data_structure/WaveletMatrix.hpp) の kth_smallest。この問題で書き比べた wm4 を写したもの。
#include "pj.hpp"
#include "neo/data_structure/WaveletMatrix.hpp"
struct Solver {
 WaveletMatrix<int> wm;
 explicit Solver(const vector<int>& a): wm(a) {}
 int kth_smallest(int l, int r, int k) const { return wm.kth_smallest(l, r, k); }
};
