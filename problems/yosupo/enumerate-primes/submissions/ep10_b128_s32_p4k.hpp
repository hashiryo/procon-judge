#pragma once
// 素数の列挙の 10 回目 (核は _ep10.hpp)。run<16384, 4096, 4000, 163, 16> (区画の語数、小区画の語数、小区画で消す素数の上限、型で消す素数の上限、2 段で取り出す A の下限)。
#include "_ep10.hpp"

inline std::pair<u32, std::vector<u32>> run(u32 N, u32 A, u32 B) { return ep10::run<16384, 4096, 4000, 163, 16>(N, A, B); }
