#pragma once
// Deléglise–Rivat の 4 回目 (核は _dr4.hpp) を α = 8、β = 1.5 で回す。3 回目から、区間ごとの語の累積を
// 4 語ずつ数えてから足す形にした。
#include "_dr4.hpp"

inline u64 run(u64 N) { return dr4::prime_pi(N, 8.0, 1.5); }
