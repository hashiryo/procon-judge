#pragma once
// Deléglise–Rivat の 10 回目 (核は _dr10.hpp)。いつも gather の道 (1) を通す (比べる用)。
#include "../common.hpp"
#include "_dr10.hpp"

inline u64 run(u64 N) { return dr10::prime_pi<1>(N); }
