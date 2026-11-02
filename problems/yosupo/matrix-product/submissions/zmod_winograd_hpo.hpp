#pragma once
// zmod_strassen_hpo の 7 回の積を Winograd の形 (1 段の足し算と引き算が 15 回) にした版 (_strassen_hp.hpp の 1、2、3)。
#include "_strassen_hp.hpp"
inline vector<u32> run(int N, int M, int P, const vector<u32>& a, const vector<u32>& b) { return strassen_hp::matmul<64, true, true>(N, M, P, a, b); }
