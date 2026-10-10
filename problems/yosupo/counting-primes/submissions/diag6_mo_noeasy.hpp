#pragma once
// 診断用 (答えは合わない)。dr6_mo から easy leaves の素数の q の表引きを抜く。diag6_mg_noeasy との差が奇数だけの表を作る手間。
#define DR6_DIAG_NOEASY
#include "_dr6.hpp"

inline u64 run(u64 N) { return dr6::prime_pi<false, true>(N); }
