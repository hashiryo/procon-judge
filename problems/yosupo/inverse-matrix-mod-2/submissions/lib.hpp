#pragma once
// Library の LU_Decomposition (Matrix<bool> は 128 bit 詰め) をハーネスの形に合わせる。
#include "../common.hpp"
#include "mylib/algebra/LU_Decomposition.hpp"
struct Inv {
 static vector<string> run(int N, const vector<string>& a) {
  Matrix<bool> A(N, N);
  for(int i= 0; i < N; ++i)
   for(int j= 0; j < N; ++j) A[i][j]= a[i][j] - '0';
  auto B= LU_Decomposition(A).inverse_matrix();
  if(!B) return {};
  vector<string> ret(N, string(N, '0'));
  for(int i= 0; i < N; ++i)
   for(int j= 0; j < N; ++j) ret[i][j]= '0' + B[i][j];
  return ret;
 }
};
