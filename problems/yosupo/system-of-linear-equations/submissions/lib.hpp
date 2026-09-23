#pragma once
// Library の LU_Decomposition の linear_equations と kernel をハーネスの形に合わせる。
#include "../common.hpp"
#include "mylib/algebra/LU_Decomposition.hpp"
#include "mylib/algebra/ModInt.hpp"
struct Solver {
 static SolveResult run(int N, int M, const vector<vector<u32>>& A_, const vector<u32>& b_) {
  using Mint= ModInt<998244353>;
  Matrix<Mint> A(N, M);
  Vector<Mint> b(N);
  for(int i= 0; i < N; ++i)
   for(int j= 0; j < M; ++j) A[i][j]= Mint(A_[i][j]);
  for(int i= 0; i < N; ++i) b[i]= Mint(b_[i]);
  LU_Decomposition lu(A);
  auto res= lu.linear_equations(b);
  if(!res) return {false, {}, {}};
  SolveResult ret{true, vector<u32>(M), {}};
  for(int j= 0; j < M; ++j) ret.sol[j]= res[j].val();
  for(const auto& v: lu.kernel()) {
   vector<u32> row(M);
   for(int j= 0; j < M; ++j) row[j]= v[j].val();
   ret.basis.push_back(row);
  }
  return ret;
 }
};
