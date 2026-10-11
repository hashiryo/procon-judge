#pragma once
// 素数の列挙の 4 回目 (核は 3 回目の _ep3.hpp)。区画の大きさを振る。run<32768, 101, false> (区画の語数、型で消す素数の上限、210 の車輪か)。
#include "_ep3.hpp"

inline std::pair<u32, std::vector<u32>> run(u32 N, u32 A, u32 B) { return ep3::run<32768, 101, false>(N, A, B); }
