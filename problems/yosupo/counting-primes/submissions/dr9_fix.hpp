#pragma once
// Deléglise–Rivat の 9 回目 (核は _dr9.hpp)。dr9_f3 に加え、P2 を AVX2 で回す。
#include "../common.hpp"
#include "_dr9.hpp"

inline u64 run(u64 N) { return dr9::prime_pi<7>(N); }
