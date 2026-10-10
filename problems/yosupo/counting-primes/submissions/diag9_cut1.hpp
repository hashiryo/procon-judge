#pragma once
// 診断用 (答えは合わない)。dr9 を篩の走査の前 (素数、合成数、逆数、hard leaves の列を用意したところ) で打ち切る。
#define DR9_CUT 1
#include "../common.hpp"
#include "_dr9.hpp"

inline u64 run(u64 N) { return dr9::prime_pi<0>(N); }
