#pragma once
// 診断用 (答えは合わない)。dr9 を篩の走査 (hard leaves と π の表) の後で打ち切る。
#define DR9_CUT 2
#include "../common.hpp"
#include "_dr9.hpp"

inline u64 run(u64 N) { return dr9::prime_pi<0>(N); }
