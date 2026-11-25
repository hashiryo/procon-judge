#pragma once
// 愚直解: 定義どおりの DFT (自然な順) を O(n^2) で求める (crosscheck 用)。
#include "_shared/ntt/_naive.hpp"
struct Kernel {
 static void dft(u32* a, int n) { ntt_naive::dft(a, n, 998244353); }
 static void idft(u32* a, int n) { ntt_naive::idft(a, n, 998244353); }
};
