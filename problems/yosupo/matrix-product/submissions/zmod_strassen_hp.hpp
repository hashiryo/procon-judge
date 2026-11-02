#pragma once
// zmod_strassen の作業の配列と答えの vector に huge page を頼む版 (_strassen_hp.hpp の 1)。核と足し算の並べ方は同じ。
#include "_strassen_hp.hpp"
inline vector<u32> run(int N, int M, int P, const vector<u32>& a, const vector<u32>& b) { return strassen_hp::matmul<64, false, false>(N, M, P, a, b); }
