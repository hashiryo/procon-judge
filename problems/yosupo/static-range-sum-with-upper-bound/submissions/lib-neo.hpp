#pragma once
// NeoLibrary の WaveletMatrixSum (neo/data_structure/WaveletMatrixSum.hpp) の count_sum_le。この問題で書き比べた
// wm_sep_brtbl を写したもの。
#include "pj.hpp"
#include "neo/data_structure/WaveletMatrixSum.hpp"
struct Solver {
 WaveletMatrixSum<int> wm;
 explicit Solver(const vector<int>& a): wm(a) {}
 pair<int, i64> count_sum(int l, int r, int x) const { return wm.count_sum_le(l, r, x); }
};
