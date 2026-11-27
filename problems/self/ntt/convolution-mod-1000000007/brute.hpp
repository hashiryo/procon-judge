#pragma once
// 愚直解: 定義どおりの二重ループ (crosscheck 用)。
#include "_shared/ntt/_naive.hpp"
struct Kernel {
 static void convolve_range(const u32* a, int n, const u32* b, int m, int lo, int hi, u32* c) {
  ntt_naive::convolve_range(a, n, b, m, lo, hi, c, 1000000007);
 }
};
