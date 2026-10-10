#pragma once
// NeoLibrary の inv_gcd (Stein の方法の拡張版を Kaliski の almost inverse の形で書いたもの) を呼ぶ。
#include "_shared/modulo-test/_common.hpp"
#include "neo/number_theory/inv_gcd.hpp"
inline std::pair<u64, u64> run(u64 a, u64 b) { return inv_gcd(a, b); }
