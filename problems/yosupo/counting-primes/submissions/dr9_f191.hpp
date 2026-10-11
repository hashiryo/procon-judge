#pragma once
// Deléglise–Rivat の 9 回目 (核は _dr9.hpp)。dr9_f31 に加え、合成数の列を印のビット列から作り、hard leaves の語ごとの累積を 4 語ずつ作る。
#include "../common.hpp"
#include "_dr9.hpp"

inline u64 run(u64 N) { return dr9::prime_pi<191>(N); }
