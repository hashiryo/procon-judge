#pragma once
// Deléglise–Rivat の 9 回目 (核は _dr9.hpp)。dr9_fix に加え、AVX2 のとき素数での割り算をすべて 1 / p の double で行い、magic を作らない。
#include "../common.hpp"
#include "_dr9.hpp"

inline u64 run(u64 N) { return dr9::prime_pi<15>(N); }
