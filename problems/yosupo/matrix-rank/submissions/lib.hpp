#pragma once
// Library の LU_Decomposition::rank をハーネスの形に合わせる。
#include "../common.hpp"
#include "mylib/algebra/LU_Decomposition.hpp"
#include "mylib/algebra/ModInt.hpp"
inline int run(int N, int M, const vector<vector<u32>>& a) {
 using Mint= ModInt<998244353>;
 Matrix<Mint> A(N, M);
 for(int i= 0; i < N; ++i)
  for(int j= 0; j < M; ++j) A[i][j]= Mint(a[i][j]);
 return LU_Decomposition(A).rank();
}
