#pragma once
// 診断用 (答えは合わない)。dr7_avx から hard leaves の語ごとの累積を抜く。dr7_avx との差が累積の時間。
#define DR7_DIAG_NOPRE
#include "_dr7.hpp"

inline u64 run(u64 N) { return dr7::prime_pi<1>(N); }
