#pragma once
// Deléglise–Rivat の 3 回目 (核は _dr3.hpp) を α = 6、β = 2 で回す。y = α x^{1/3}、w = β y。
#include "_dr3.hpp"

inline u64 run(u64 N) { return dr3::prime_pi(N, 6.0, 2); }
