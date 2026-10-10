#pragma once
// Deléglise–Rivat の 2 回目 (核は _dr2.hpp) を α = 10 で回す。y = α x^{1/3}。
#include "_dr2.hpp"

inline u64 run(u64 N) { return dr2::prime_pi(N, 10.0); }
