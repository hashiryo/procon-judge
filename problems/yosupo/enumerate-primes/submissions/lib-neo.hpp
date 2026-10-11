#pragma once
// NeoLibrary の enumerate_primes を呼ぶ。中身は 12 回目 (_ep12.hpp) の b32_s32_p4k の組を、区画と小区画を 1 つにまとめて
// 写したもの (30 の車輪のビット、163 までの素数の型、周を丸ごと消して後ろのはみ出しを持ち越す形、4000 以下の素数は 210 の
// 車輪、取り出しは 2 段)。lib.hpp は今の Library の奇数だけの篩。
#include "../common.hpp"
#include "neo/number_theory/enumerate_primes.hpp"

inline std::pair<u32, std::vector<u32>> run(u32 N, u32 A, u32 B) { return enumerate_primes(N, A, B); }
