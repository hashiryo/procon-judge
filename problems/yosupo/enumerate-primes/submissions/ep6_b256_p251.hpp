#pragma once
// 素数の列挙の 6 回目 (核は _ep6.hpp)。run<32768, 2048, 1000, 251> (区画の語数、小区画の語数、小区画で消す素数の上限、型で消す素数の上限)。
#include "_ep6.hpp"

inline std::pair<u32, std::vector<u32>> run(u32 N, u32 A, u32 B) { return ep6::run<32768, 2048, 1000, 251>(N, A, B); }
