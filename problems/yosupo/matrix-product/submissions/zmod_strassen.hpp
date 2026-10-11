#pragma once
// 64 × 64 のブロックの核 (4 行 × 8 列のタイル、AVX2) の上に Strassen の再帰を重ねた行列積 (_strassen.hpp)。法は 998244353。
#include "_strassen.hpp"
inline vector<u32> run(int N, int M, int P, const vector<u32>& a, const vector<u32>& b) { return strassen::matmul(N, M, P, a, b); }
