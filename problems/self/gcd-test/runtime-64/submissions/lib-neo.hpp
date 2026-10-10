#pragma once
// NeoLibrary の gcd (Stein の方法に、桁数の差が大きいときだけ割り算 1 回を混ぜたもの) を呼ぶ。
// lib-binary_gcd.hpp は今の Library (mylib) の binary_gcd。
#include "_shared/modulo-test/_common.hpp"
#include "neo/number_theory/gcd.hpp"
inline u64 run(u64 a, u64 b) { return gcd(a, b); }
