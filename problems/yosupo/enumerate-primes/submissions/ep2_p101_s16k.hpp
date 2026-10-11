#pragma once
// 素数の列挙の 2 回目 (核は _ep2.hpp)。run<2048, 101> (区画の語数、型で消す素数の上限)。
#include "_ep2.hpp"

inline std::pair<u32, std::vector<u32>> run(u32 N, u32 A, u32 B) { return ep2::run<2048, 101>(N, A, B); }
