#pragma once
// zmod_strassen_hp の核で、B の行の奇数番目の列を 4 byte ずらした番地から読み直す版 (_strassen_hp.hpp の 1 と 2)。
#include "_strassen_hp.hpp"
inline vector<u32> run(int N, int M, int P, const vector<u32>& a, const vector<u32>& b) { return strassen_hp::matmul<64, false, true>(N, M, P, a, b); }
