#pragma once
// 2 分の動的 wavelet matrix。各段は 64 bit の塊を持つ treap の動的ビット列。
// 値は 0 以上 2^17 未満 (この問題の値は 0..70000)。
#include "_shared/wavelet/dwm.hpp"

struct Solver {
  static constexpr int L = 17;
  dwm_internal::DynamicWaveletMatrix<L> wm;

  explicit Solver(const vector<int> &a) : wm(a) {}

  void insert(int k, int x) { wm.insert(k, x); }
  void set(int k, int x) { wm.set(k, x); }
  int kth(int l, int r, int k) const { return wm.kth(l, r, k); }
};
