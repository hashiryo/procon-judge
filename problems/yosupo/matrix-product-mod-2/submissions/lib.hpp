#pragma once
// Library の Matrix<bool> の積をハーネスの形に合わせる。
#include "../common.hpp"
#include "mylib/algebra/Matrix.hpp"
inline vector<string> run(int N, int M, int K, const vector<string>& a, const vector<string>& b) {
 Matrix<bool> A(N, M), B(M, K);
 for(int i= 0; i < N; ++i)
  for(int j= 0; j < M; ++j) A[i][j]= a[i][j] - '0';
 for(int i= 0; i < M; ++i)
  for(int j= 0; j < K; ++j) B[i][j]= b[i][j] - '0';
 auto C= A * B;
 vector<string> ret(N, string(K, '0'));
 for(int i= 0; i < N; ++i)
  for(int j= 0; j < K; ++j) ret[i][j]= '0' + C[i][j];
 return ret;
}
