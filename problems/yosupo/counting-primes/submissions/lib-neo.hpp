#pragma once
// NeoLibrary の prime_pi (Deléglise–Rivat) を呼ぶ。中身は _dr5.hpp を w = y にして、倍数を消すときに消えた数を
// 局所変数で数える形にしたもの。lib.hpp は今の Library の Lucy の DP。
#include "../common.hpp"
#include "neo/number_theory/prime_pi.hpp"

inline u64 run(u64 N) { return prime_pi(N); }
