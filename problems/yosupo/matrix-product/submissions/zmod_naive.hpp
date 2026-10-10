#pragma once
// NeoLibrary の ZMod<998244353> の演算子だけで、i-k-j の順に積を 1 つずつ還元する (_zmod_mat.hpp の試作)。
#include "_zmod_mat.hpp"
inline vector<u32> run(int N, int M, int P, const vector<u32>& a, const vector<u32>& b) {
 return run_zmod(N, M, P, a, b, [](auto... x) { return mat_mul_naive(x...); });
}
