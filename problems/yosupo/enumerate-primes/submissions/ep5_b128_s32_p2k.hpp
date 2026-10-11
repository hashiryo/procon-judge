#pragma once
// 素数の列挙の 5 回目 (核は _ep5.hpp)。run<16384, 4096, 2000> (区画の語数、小区画の語数、小区画で消す素数の上限)。
#include "_ep5.hpp"

inline std::pair<u32, std::vector<u32>> run(u32 N, u32 A, u32 B) { return ep5::run<16384, 4096, 2000>(N, A, B); }
