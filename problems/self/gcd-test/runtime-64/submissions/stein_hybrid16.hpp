#pragma once
// 桁数の差が 16 bit を超えるときだけ、大きいほうを小さいほうで 1 回割ってから Stein に入る。
#include "_stein.hpp"
inline u64 run(u64 a, u64 b) { return gcd_div1<16>(a, b); }
