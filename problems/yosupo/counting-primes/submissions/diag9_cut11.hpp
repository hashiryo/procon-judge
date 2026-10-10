#pragma once
// 診断用 (答えは合わない)。dr9 を。
#define DR9_CUT 11
#include "../common.hpp"
#include "_dr9.hpp"

inline u64 run(u64 N) { return dr9::prime_pi<0>(N); }
