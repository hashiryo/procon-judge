#pragma once
// NeoLibrary の動的 wavelet matrix。値は 1 以上 N 以下 (N <= 100000) なので 2^17 未満。
#include "neo/base/ds/range/DynamicWaveletMatrix.hpp"

struct Solver {
  DynamicWaveletMatrix<unsigned, 17> wm;

  static DynamicWaveletMatrix<unsigned, 17> build(const vector<int> &a) {
    vector<unsigned> b(a.begin(), a.end());
    return DynamicWaveletMatrix<unsigned, 17>(std::move(b));
  }

  explicit Solver(const vector<int> &a) : wm(build(a)) {}

  void shift(int l, int r) {
    unsigned x = wm.erase(r - 1);
    wm.insert(l, x);
  }

  int count(int l, int r, int k) const { return wm.freq(l, r, (unsigned)k); }
};
