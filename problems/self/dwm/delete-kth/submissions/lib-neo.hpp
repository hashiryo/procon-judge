#pragma once
// NeoLibrary の動的 wavelet matrix。値は 1 以上 10^9 以下なので 2^30 未満。
#include "neo/base/ds/range/DynamicWaveletMatrix.hpp"

struct Solver {
  DynamicWaveletMatrix<unsigned, 30> wm;

  Solver() = default;

  void push_back(int x) { wm.insert(wm.size(), (unsigned)x); }
  void erase(int k) { wm.erase(k); }
  void set(int k, int x) { wm.set(k, (unsigned)x); }
  int kth(int l, int r, int k) const { return (int)wm.kth_smallest(l, r, k); }
};
