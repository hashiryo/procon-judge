#pragma once
// 愚直解: 定義どおりの二重ループ (crosscheck 用)。二項係数はパスカルの漸化式で作る。
#include "_shared/ntt/_reference_hurwitz.hpp"
struct Kernel {
 static void convolve(const u32* a, int n, const u32* b, int m, u32* c, u32 mod) {
  hurwitz_reference::naive_binom(a, n, b, m, c, mod);
 }
};
