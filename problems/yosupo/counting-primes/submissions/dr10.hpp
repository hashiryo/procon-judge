#pragma once
// Deléglise–Rivat の 10 回目 (核は _dr10.hpp)。CPU を見て道を選ぶ (NeoLibrary の次の版の候補)。
#include "../common.hpp"
#include "_dr10.hpp"

inline u64 run(u64 N) { return dr10::prime_pi<0>(N); }
