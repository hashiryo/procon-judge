#pragma once
// Deléglise–Rivat の 9 回目 (核は _dr9.hpp)。NeoLibrary 0e2f14f の prime_pi の写しそのもの。
#include "../common.hpp"
#include "_dr9.hpp"

inline u64 run(u64 N) { return dr9::prime_pi<0>(N); }
