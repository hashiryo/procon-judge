#pragma once
// Library の binary_gcd (分岐の無い Stein のアルゴリズム) を u32 で呼ぶ。
#include "_shared/modulo-test/_common.hpp"
#include "mylib/number_theory/binary_gcd.hpp"
inline u32 run(u32 a, u32 b) { return binary_gcd(a, b); }
