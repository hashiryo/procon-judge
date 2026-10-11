#pragma once
// Deléglise–Rivat の 9 回目 (核は _dr9.hpp)。dr9_f15 に加え、大きい配列のページを 0 で埋める前に MADV_POPULATE_WRITE でまとめて用意させる。
#include "../common.hpp"
#include "_dr9.hpp"

inline u64 run(u64 N) { return dr9::prime_pi<31>(N); }
