#pragma once
// 診断用。zmod_naive と同じ i-k-j のループを、ZMod を通さずに u32 の配列と ZMod と同じ式の Barrett で書く (_diag_naive.hpp)。
#include "_diag_naive.hpp"
inline vector<u32> run(int N, int M, int P, const vector<u32>& a, const vector<u32>& b) { return run_diag<DiagBarrett>(N, M, P, a, b); }
