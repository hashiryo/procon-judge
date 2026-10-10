#pragma once
// NeoLibrary の ZMod<998244353> で、和を 4 行 × 8 列のタイルとしてレジスタに持ち、AVX2 で 8 段ごとに畳む (_zmod_mat_avx2.hpp の試作)。
#include "_zmod_mat.hpp"
#include "_zmod_mat_avx2.hpp"
inline vector<u32> run(int N, int M, int P, const vector<u32>& a, const vector<u32>& b) {
 return run_zmod(N, M, P, a, b, [](auto... x) { return mat_mul_avx2(x...); });
}
