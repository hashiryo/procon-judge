#pragma once
// NeoLibrary の inv_gcd を u32 で呼ぶ。32 bit の入力では 2^{-k} の還元が 1 回で済む。
#include "_shared/modulo-test/_common.hpp"
#include "neo/number_theory/inv_gcd.hpp"
inline std::pair<u32, u32> run(u32 a, u32 b) { return inv_gcd(a, b); }
