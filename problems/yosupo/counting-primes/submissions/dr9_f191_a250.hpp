#pragma once
// Deléglise–Rivat の 9 回目 (核は _dr9.hpp)。dr9_f191 の α を primecount の当てはめの 2.5 倍にする (dr9_f191 は 1.5 倍)。
#include "../common.hpp"
#include "_dr9.hpp"

inline u64 run(u64 N) { return dr9::prime_pi<191, 250>(N); }
