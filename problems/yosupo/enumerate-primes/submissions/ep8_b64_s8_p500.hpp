#pragma once
// 素数の列挙の 8 回目 (核は _ep8.hpp)。run<8192, 1024, 500, 163, 64> (区画の語数、小区画の語数、小区画で消す素数の上限、型で消す素数の上限、分岐なしで取り出す A の上限)。
#include "_ep8.hpp"

inline std::pair<u32, std::vector<u32>> run(u32 N, u32 A, u32 B) { return ep8::run<8192, 1024, 500, 163, 64>(N, A, B); }
