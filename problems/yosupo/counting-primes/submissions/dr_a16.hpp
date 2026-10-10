#pragma once
// Deléglise–Rivat (核は _dr.hpp) を α = 16 で回す。y = α x^{1/3}。
#include "_dr.hpp"

inline u64 run(u64 N) { return dr::prime_pi(N, 16.0); }
