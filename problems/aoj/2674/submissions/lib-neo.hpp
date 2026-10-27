#pragma once
// NeoLibrary の WaveletMatrix (neo/data_structure/WaveletMatrix.hpp) の count。この問題で書き比べた wm4 を写したもの。
#include "pj.hpp"
#include "neo/data_structure/WaveletMatrix.hpp"
struct Solver {
 WaveletMatrix<int> wm;
 explicit Solver(const vector<int>& x): wm(x) {}
 int count(int l, int r, int lo, int hi) const { return wm.count(l, r, lo, hi); }
};
