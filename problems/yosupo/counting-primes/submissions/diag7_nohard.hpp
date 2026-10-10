#pragma once
// 診断用 (答えは合わない)。dr7_avx から hard leaves の累積と葉を両方抜く。diag7_nopre との差が葉を数える時間。
#define DR7_DIAG_NOHARD
#include "_dr7.hpp"

inline u64 run(u64 N) { return dr7::prime_pi<1>(N); }
