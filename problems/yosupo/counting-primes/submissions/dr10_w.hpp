#pragma once
// Deléglise–Rivat の 10 回目 (核は _dr10.hpp)。いつも車輪の表に AVX2 で商と位置だけ求める道 (2) を通す (比べる用)。
#include "../common.hpp"
#include "_dr10.hpp"

inline u64 run(u64 N) { return dr10::prime_pi<2>(N); }
