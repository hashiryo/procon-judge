#pragma once
// zmod_winograd_hpo の再帰を 1 段深くし、核を 1 辺 32 のブロックにした版。1024 次の積和は 7^5 32^3 回 (64 の核の 7/8) になり、
// 足し算と引き算が 1 段分増える。
#include "_strassen_hp.hpp"
inline vector<u32> run(int N, int M, int P, const vector<u32>& a, const vector<u32>& b) { return strassen_hp::matmul<32, true, true>(N, M, P, a, b); }
