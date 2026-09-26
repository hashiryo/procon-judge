#pragma once
// Library の LU_Decomposition::inverse_matrix をハーネスの形に合わせる。
#include "../common.hpp"
#include "mylib/algebra/LU_Decomposition.hpp"
#include "mylib/algebra/ModInt.hpp"
inline InverseResult run(int N, const vector<vector<u32>>& a) {
 using Mint= ModInt<998244353>;
 Matrix<Mint> A(N, N);
 for(int i= 0; i < N; ++i)
  for(int j= 0; j < N; ++j) A[i][j]= Mint(a[i][j]);
 auto ans= LU_Decomposition(A).inverse_matrix();
 if(!ans) return {false, {}};
 vector<vector<u32>> mat(N, vector<u32>(N));
 for(int i= 0; i < N; ++i)
  for(int j= 0; j < N; ++j) mat[i][j]= ans[i][j].val();
 return {true, mat};
}
