#pragma once
// 2 分の動的 wavelet matrix。各段は 256 bit の塊を並べ、塊の列の上に Fenwick を載せた動的ビット列。
// 値は 1 以上 10^9 以下なので、2^30 未満の順位として扱う。
#include "_shared/wavelet/dwm.hpp"

struct Solver {
  static constexpr int L = 30;
  dwm_internal::DynamicWaveletMatrix<L, dwm_internal::BlockBitVector> wm;

  Solver() = default;

  void push_back(int x) { wm.insert(wm.size(), x); }
  void erase(int k) { wm.erase(k); }
  void set(int k, int x) { wm.set(k, x); }
  int kth(int l, int r, int k) const { return wm.kth(l, r, k); }
};
