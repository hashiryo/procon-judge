#pragma once
// Deléglise–Rivat の 6 回目 (核は _dr6.hpp)。割り算は floor(2^64 / d) + 1 との掛け算の上位、π の表は 30 の車輪 (NeoLibrary の版と同じ)。
#include "_dr6.hpp"

inline u64 run(u64 N) { return dr6::prime_pi<false, false>(N); }
