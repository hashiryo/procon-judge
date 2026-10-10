#pragma once
// 診断用 (答えは合わない)。dr7_avx から ordinary leaves を抜く。
#define DR7_DIAG_NOS1
#include "_dr7.hpp"

inline u64 run(u64 N) { return dr7::prime_pi<1>(N); }
