#pragma once
// Library の Matrix<ModInt> の積をハーネスの形 (flat row-major) に合わせる。
#include "../common.hpp"
#include "mylib/algebra/Matrix.hpp"
#include "mylib/algebra/ModInt.hpp"
inline vector<u32> run(int N, int M, int P, const vector<u32>& a, const vector<u32>& b) {
 using Mint= ModInt<998244353>;
 Matrix<Mint> A(N, M), B(M, P);
 for(int i= 0; i < N; ++i)
  for(int j= 0; j < M; ++j) A[i][j]= Mint(a[(size_t)i * M + j]);
 for(int i= 0; i < M; ++i)
  for(int j= 0; j < P; ++j) B[i][j]= Mint(b[(size_t)i * P + j]);
 auto C= A * B;
 vector<u32> ret((size_t)N * P);
 for(int i= 0; i < N; ++i)
  for(int j= 0; j < P; ++j) ret[(size_t)i * P + j]= C[i][j].val();
 return ret;
}
