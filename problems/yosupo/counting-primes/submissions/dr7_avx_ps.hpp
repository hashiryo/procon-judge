#pragma once
// Deléglise–Rivat の 7 回目 (核は _dr7.hpp)。dr7_avx に加え、P2 と ordinary leaves を AVX2 で 4 つずつ回す。
#include "_dr7.hpp"

inline u64 run(u64 N) { return dr7::prime_pi<1, false, true, true>(N); }
