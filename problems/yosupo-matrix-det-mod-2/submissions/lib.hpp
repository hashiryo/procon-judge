#pragma once
// Library の LU_Decomposition (Matrix<bool>) の det をハーネスの形に合わせる。
#include "../common.hpp"
#include "mylib/algebra/LU_Decomposition.hpp"
struct Det {
 static int run(int N, const vector<string>& a) {
  Matrix<bool> A(N, N);
  for(int i= 0; i < N; ++i)
   for(int j= 0; j < N; ++j) A[i][j]= a[i][j] - '0';
  return LU_Decomposition(A).det();
 }
};
