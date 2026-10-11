#pragma once
// 今の Library の WaveletMatrix (mylib/data_structure/WaveletMatrix.hpp) で、[x, x + 1) に入る値の数を数える。
#include "pj.hpp"
#include "mylib/data_structure/WaveletMatrix.hpp"
struct Solver {
 WaveletMatrix<int> wm;
 explicit Solver(const vector<int>& a): wm(a) {}
 int freq(int l, int r, int x) const { return l < r ? wm.count(l, r, x, x + 1) : 0; }
};
