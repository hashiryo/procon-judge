#pragma once
// 重み付きの動的 WM。各段は 256 bit の塊 + Fenwick で、各 bit に値の重みを持つ。
// 点更新と、区間の大きい方から和が k 以上になる最小個数 (総和が k 未満なら -1) を返す。
// 値は 1 以上 10^9 以下なので 2^30 未満。
#include "_shared/wavelet/dwm_sum.hpp"

struct Solver {
  static constexpr int L = 30;
  dwm_sum_internal::WeightedWaveletMatrix<L> wm;

  explicit Solver(const vector<int> &a) : wm(a) {}

  void set(int c, int x) { wm.set(c, x); }
  int query(int l, int r, long long k) const { return wm.min_count_ge(l, r, k); }
};
