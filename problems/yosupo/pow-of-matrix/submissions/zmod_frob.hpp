#pragma once
// Frobenius 標準形で A^K を求める (_frob.hpp)。手間は O(N^3) と、ブロックごとの x^K mod f の計算。
#include "_frob.hpp"
inline vector<u32> run(int N, u64 K, const vector<u32>& a) { return frob::pow(N, K, a); }
