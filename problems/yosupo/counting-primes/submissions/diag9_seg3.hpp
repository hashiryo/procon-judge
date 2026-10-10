#pragma once
// 診断用 (答えは合わない)。diag9_cut2 (篩の走査の後で打ち切る) から、hard leaves のある素数の倍数を数えずに消す。diag9_cut2 との差がその処理の時間。
#define DR9_CUT 2
#define DR9_SEG 3
#include "../common.hpp"
#include "_dr9.hpp"

inline u64 run(u64 N) { return dr9::prime_pi<0>(N); }
