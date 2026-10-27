#pragma once
// NeoLibrary の WaveletMatrix (neo/data_structure/WaveletMatrix.hpp) の freq。値ごとの位置の列 (pos_tbl_cm) の方が速いが、
// ライブラリの操作として測る。
#include "pj.hpp"
#include "neo/data_structure/WaveletMatrix.hpp"
struct Solver {
 WaveletMatrix<int> wm;
 explicit Solver(const vector<int>& a): wm(a) {}
 int freq(int l, int r, int x) const { return wm.freq(l, r, x); }
};
