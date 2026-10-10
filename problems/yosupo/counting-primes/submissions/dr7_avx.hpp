#pragma once
// Deléglise–Rivat の 7 回目 (核は _dr7.hpp)。easy leaves の表引きを AVX2 で 4 つずつ回す (商は double の掛け算、表は gather)。
#include "_dr7.hpp"

inline u64 run(u64 N) { return dr7::prime_pi<1>(N); }
