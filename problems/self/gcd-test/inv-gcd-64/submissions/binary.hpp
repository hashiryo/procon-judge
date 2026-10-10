#pragma once
// 割り算を使わない Stein の拡張版 (Kaliski の almost inverse の形)。_kaliski.hpp の T = 64。
#include "_kaliski.hpp"
inline std::pair<u64, u64> run(u64 a, u64 b) { return kaliski::inv_gcd<64>(a, b); }
