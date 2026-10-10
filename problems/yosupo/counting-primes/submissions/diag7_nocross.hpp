#pragma once
// 診断用 (答えは合わない)。dr7_avx から、hard leaves の無い √z 以下の素数で倍数を消す処理 (π の表のためだけの分) を抜く。
#define DR7_DIAG_NOCROSS
#include "_dr7.hpp"

inline u64 run(u64 N) { return dr7::prime_pi<1>(N); }
