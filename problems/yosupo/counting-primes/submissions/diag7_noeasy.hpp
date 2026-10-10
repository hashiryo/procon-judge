#pragma once
// 診断用 (答えは合わない)。dr7_avx から easy leaves の素数の q の表引きを抜く。diag6_mg_noeasy との差が、奇数だけの表を pdep で作る手間。
#define DR7_DIAG_NOEASY
#include "_dr7.hpp"

inline u64 run(u64 N) { return dr7::prime_pi<1>(N); }
