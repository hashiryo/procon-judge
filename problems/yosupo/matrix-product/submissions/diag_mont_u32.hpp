#pragma once
// 診断用。zmod_naive と同じ i-k-j のループを、u32 の配列と今の Library の MP_Mo32 と同じ式の Montgomery で書く (_diag_naive.hpp)。
#include "_diag_naive.hpp"
inline vector<u32> run(int N, int M, int P, const vector<u32>& a, const vector<u32>& b) { return run_diag<DiagMont>(N, M, P, a, b); }
