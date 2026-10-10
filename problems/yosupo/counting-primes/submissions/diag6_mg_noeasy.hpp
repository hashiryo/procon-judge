#pragma once
// 診断用 (答えは合わない)。dr6_mg から easy leaves の素数の q の表引きを抜く。dr6_mg との時間の差が表引きの時間。
#define DR6_DIAG_NOEASY
#include "_dr6.hpp"

inline u64 run(u64 N) { return dr6::prime_pi<false, false>(N); }
