#pragma once
// 参照実装: ライブラリを読まない二項畳み込み (階乗で分ける道 + 素数べきごとの道 + 4 本の NTT の素数)。
#include "_shared/ntt/_reference_hurwitz.hpp"
struct Kernel {
 static void convolve(const u32* a, int n, const u32* b, int m, u32* c, u32 mod) {
  hurwitz_reference::convolve(a, n, b, m, c, mod);
 }
};
