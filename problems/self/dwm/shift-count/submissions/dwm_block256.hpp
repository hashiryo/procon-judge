#pragma once
// 2 分の動的 wavelet matrix。各段は 256 bit の塊を並べ、塊の列の上に Fenwick を載せた動的ビット列。
// 値は 1 以上 N 以下 (N <= 100000) なので 2^17 未満。
#include "_shared/wavelet/dwm.hpp"

struct Solver {
  static constexpr int L = 17;
  dwm_internal::DynamicWaveletMatrix<L, dwm_internal::BlockBitVector> wm;

  explicit Solver(const vector<int> &a) : wm(a) {}

  void shift(int l, int r) {
    int x = wm.access(r - 1);
    wm.erase(r - 1);
    wm.insert(l, x);
  }

  int count(int l, int r, int k) const { return wm.count(l, r, k); }
};
