#pragma once
// 組の部分集合の DP を最大の組ごとの層に分け、配る値を AVX2 で列の向きに 4 個ずつ求める版 (_layer.hpp)。頂点ごとの配列は
// 部分集合の番号でそのまま引く (半分は使わない)。法は 998244353。
#include "_layer.hpp"
inline u32 run(int N, const vector<vector<u32>>& a) { return hafnian_layer::hafnian<false>(N, a); }
