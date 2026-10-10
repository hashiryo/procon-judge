#pragma once
// NeoLibrary の ZMod<998244353> で、B を転置し、内積の積を 16 個ずつ u64 にためて還元する (_zmod_mat.hpp の試作)。
#include "_zmod_mat.hpp"
inline vector<u32> run(int N, int M, int P, const vector<u32>& a, const vector<u32>& b) {
 return run_zmod(N, M, P, a, b, [](auto... x) { return mat_mul_dot16(x...); });
}
