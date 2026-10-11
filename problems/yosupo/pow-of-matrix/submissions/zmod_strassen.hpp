#pragma once
// 二分累乗。積は _strassen.hpp の Strassen の再帰 (64 × 64 の核の上) で、1 辺を 2 の冪に広げて Morton 順のまま掛け続ける。
#include "_strassen.hpp"
inline vector<u32> run(int N, u64 K, const vector<u32>& a) { return strassen::pow(N, K, a); }
