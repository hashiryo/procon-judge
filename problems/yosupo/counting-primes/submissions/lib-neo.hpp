#pragma once
// NeoLibrary の prime_pi (Deléglise–Rivat) を呼ぶ。中身は 8 回目 (procon-judge eed040d6 の _dr8.hpp) と同じで、AVX2 が
// あれば easy leaves の表引きと ordinary leaves を 4 つずつ回す。lib.hpp は今の Library の Lucy の DP。
#include "../common.hpp"
#include "neo/number_theory/prime_pi.hpp"

inline u64 run(u64 N) { return prime_pi(N); }
