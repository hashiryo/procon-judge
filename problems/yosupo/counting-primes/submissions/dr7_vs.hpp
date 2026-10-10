#pragma once
// Deléglise–Rivat の 7 回目 (核は _dr7.hpp)。easy leaves の商だけ AVX2 で 4 つずつ求め、奇数だけの表は 1 つずつ引く。
#include "_dr7.hpp"

inline u64 run(u64 N) { return dr7::prime_pi<2>(N); }
