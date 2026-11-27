#pragma once
// 参照実装: ライブラリを読まない CRT (3 つの NTT の素数で _reference.hpp を回して Garner で戻す)。
#include "_shared/ntt/_reference_crt.hpp"
struct Kernel {
 static void convolve_range(const u32* a, int n, const u32* b, int m, int lo, int hi, u32* c) {
  ntt_reference_crt::convolve_range(a, n, b, m, lo, hi, c, 1000000007);
 }
};
