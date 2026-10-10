#pragma once
// Stein の拡張版に、桁数の差が 16 bit を超えるときだけ割り算 1 回 (A %= B か互除法の 1 段) を混ぜる。
#include "_kaliski.hpp"
inline std::pair<u64, u64> run(u64 a, u64 b) { return kaliski::inv_gcd<16>(a, b); }
