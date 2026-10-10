#pragma once
// Deléglise–Rivat の 7 回目 (核は _dr7.hpp)。dr7_avx に加え、hard leaves の素数の q の葉も商と位置を AVX2 で 4 つずつ求める。
#include "_dr7.hpp"

inline u64 run(u64 N) { return dr7::prime_pi<1, true>(N); }
