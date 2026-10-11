#pragma once
// 素数の列挙の 1 回目 (核は _ep1.hpp)。区画は 4096 語 (32 KB)。
#include "_ep1.hpp"

inline std::pair<u32, std::vector<u32>> run(u32 N, u32 A, u32 B) { return ep1::run<4096>(N, A, B); }
