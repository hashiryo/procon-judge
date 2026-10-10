#pragma once
// 診断用で、答えは合わない。dr4_a8_b15 から hard leaves と、数えながら消す処理を抜き、時間の差で x64 の段ごとの時間を見る。
#define DR_DIAG 3
#include "_dr4.hpp"

inline u64 run(u64 N) { return dr4::prime_pi(N, 8.0, 1.5); }
