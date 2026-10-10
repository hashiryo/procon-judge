#pragma once
#include "_shared/modulo-test/_common.hpp"
#include <numeric>
// 標準ライブラリの std::gcd。
inline u32 run(u32 a, u32 b) { return std::gcd(a, b); }
