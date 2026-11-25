#pragma once
// 参照実装: ライブラリを読まない NTT の畳み込み。全体の積を求めて範囲を切り出す。
#include "_shared/ntt/_reference.hpp"
struct Kernel {
 static void convolve_range(const u32* a, int n, const u32* b, int m, int lo, int hi, u32* c) {
  ntt_reference::convolve_range<998244353>(a, n, b, m, lo, hi, c);
 }
};
