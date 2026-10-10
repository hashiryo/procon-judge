#pragma once
// Deléglise–Rivat の 5 回目 (核は _dr5.hpp) を α = 8、β = 1.5 で回す。
#include "_dr5.hpp"

inline u64 run(u64 N) { return dr5::prime_pi(N, 8.0, 1.5); }
