#pragma once
// Deléglise–Rivat の 6 回目 (核は _dr6.hpp)。割り算は 1 / d を 1 ulp 上げた double との掛け算、π の表は 30 の車輪。
#include "_dr6.hpp"

inline u64 run(u64 N) { return dr6::prime_pi<true, false>(N); }
