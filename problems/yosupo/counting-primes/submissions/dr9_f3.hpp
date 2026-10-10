#pragma once
// Deléglise–Rivat の 9 回目 (核は _dr9.hpp)。b ごとの配列を小さく取り、φ(t, 6) の表を 7、11、13 の型から作る。
#include "../common.hpp"
#include "_dr9.hpp"

inline u64 run(u64 N) { return dr9::prime_pi<3>(N); }
