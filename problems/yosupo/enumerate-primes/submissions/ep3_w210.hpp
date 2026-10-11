#pragma once
// 素数の列挙の 3 回目 (核は _ep3.hpp)。run<4096, 101, true> (区画の語数、型で消す素数の上限、210 の車輪で倍数を消すか)。
#include "_ep3.hpp"

inline std::pair<u32, std::vector<u32>> run(u32 N, u32 A, u32 B) { return ep3::run<4096, 101, true>(N, A, B); }
