#pragma once
// 素数の列挙の 11 回目 (核は _ep11.hpp)。run<8192, 8192, 9000, 163> (区画の語数、小区画の語数、小区画で消す素数の上限、型で消す素数の上限)。
#include "_ep11.hpp"

inline std::pair<u32, std::vector<u32>> run(u32 N, u32 A, u32 B) { return ep11::run<8192, 8192, 9000, 163>(N, A, B); }
