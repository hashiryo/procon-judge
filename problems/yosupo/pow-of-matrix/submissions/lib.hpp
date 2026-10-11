#pragma once
// Library の Matrix<ModInt>::pow (二分累乗) をハーネスの形 (行優先の 1 本の配列) に合わせる。
#include "../common.hpp"
#include "mylib/algebra/Matrix.hpp"
#include "mylib/algebra/ModInt.hpp"
inline vector<u32> run(int N, u64 K, const vector<u32>& a) {
 using Mint= ModInt<998244353>;
 Matrix<Mint> A(N, N);
 for(int i= 0; i < N; ++i)
  for(int j= 0; j < N; ++j) A[i][j]= Mint(a[(size_t)i * N + j]);
 auto C= A.pow(K);
 vector<u32> ret((size_t)N * N);
 for(int i= 0; i < N; ++i)
  for(int j= 0; j < N; ++j) ret[(size_t)i * N + j]= C[i][j].val();
 return ret;
}
