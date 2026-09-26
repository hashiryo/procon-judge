#pragma once
// Library の LU_Decomposition (Matrix<bool>) の rank をハーネスの形に合わせる。
// 行が列より多ければ転置してから (Library の test と同じ)。
#include "../common.hpp"
#include "mylib/algebra/LU_Decomposition.hpp"
inline int run(int N, int M, const vector<string>& a) {
 if(N <= M) {
  Matrix<bool> A(N, M);
  for(int i= 0; i < N; ++i)
   for(int j= 0; j < M; ++j) A[i][j]= a[i][j] - '0';
  return LU_Decomposition(A).rank();
 }
 Matrix<bool> A(M, N);
 for(int i= 0; i < N; ++i)
  for(int j= 0; j < M; ++j) A[j][i]= a[i][j] - '0';
 return LU_Decomposition(A).rank();
}
