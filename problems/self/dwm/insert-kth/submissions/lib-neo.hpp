#pragma once
// NeoLibrary の動的 wavelet matrix。値は 0 以上 2^17 未満 (この問題の値は 0..70000)。
#include "neo/base/ds/range/DynamicWaveletMatrix.hpp"

struct Solver {
  DynamicWaveletMatrix<unsigned, 17> wm;

  static DynamicWaveletMatrix<unsigned, 17> build(const vector<int> &a) {
    vector<unsigned> b(a.begin(), a.end());
    return DynamicWaveletMatrix<unsigned, 17>(std::move(b));
  }

  explicit Solver(const vector<int> &a) : wm(build(a)) {}

  void insert(int k, int x) { wm.insert(k, (unsigned)x); }
  void set(int k, int x) { wm.set(k, (unsigned)x); }
  int kth(int l, int r, int k) const { return (int)wm.kth_smallest(l, r, k); }
};
