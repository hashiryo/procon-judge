#pragma once
// NeoLibrary の prime_pi (Deléglise–Rivat) を呼ぶ。中身は 10 回目 (procon-judge 04681528 の _dr10.hpp) と同じで、CPU を
// 見て、gather の遅い Intel では 30 の車輪の π の表を 1 つずつ引き、それ以外は AVX2 の gather で引く。lib.hpp は今の
// Library の Lucy の DP。
#include "../common.hpp"
#include "neo/number_theory/prime_pi.hpp"

inline u64 run(u64 N) { return prime_pi(N); }
