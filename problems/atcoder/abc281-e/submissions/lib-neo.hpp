#pragma once
// NeoLibrary の WaveletMatrixSum (neo/data_structure/WaveletMatrixSum.hpp) の sum_smallest。この問題で書き比べた wm_sep を
// 写したもの。
#include "pj.hpp"
#include "neo/data_structure/WaveletMatrixSum.hpp"
struct Solver {
 WaveletMatrixSum<int> wm;
 explicit Solver(const vector<int>& a): wm(a) {}
 i64 sum_smallest(int l, int r, int k) const { return wm.sum_smallest(l, r, k); }
};
