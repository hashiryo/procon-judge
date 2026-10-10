#pragma once
// NeoLibrary の gcd を u32 で呼ぶ (中も 32 bit の型で回り、割り算も 32 bit になる)。
#include "_shared/modulo-test/_common.hpp"
#include "neo/number_theory/gcd.hpp"
inline u32 run(u32 a, u32 b) { return gcd(a, b); }
