#pragma once
// 素数の列挙の 7 回目 (核は _ep7.hpp)。run<16384, 4096, 4000, 163, 64> (区画の語数、小区画の語数、小区画で消す素数の上限、型で消す素数の上限、分岐なしで取り出す A の上限)。
#include "_ep7.hpp"

inline std::pair<u32, std::vector<u32>> run(u32 N, u32 A, u32 B) { return ep7::run<16384, 4096, 4000, 163, 64>(N, A, B); }
