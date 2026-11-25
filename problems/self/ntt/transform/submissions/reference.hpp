#pragma once
// 参照実装: ライブラリを読まない素朴な NTT (DIF と DIT)。期待出力を作る。
#include "_shared/ntt/_reference.hpp"
struct Kernel {
 static void dft(u32* a, int n) { ntt_reference::dft<998244353>(a, n); }
 static void idft(u32* a, int n) { ntt_reference::idft<998244353>(a, n); }
};
