#pragma once
// Library の LU_Decomposition::det をハーネスの形に合わせる。
#include "../common.hpp"
#include "mylib/algebra/LU_Decomposition.hpp"
#include "mylib/algebra/ModInt.hpp"
struct Det {
 static u32 run(int n, const vector<vector<u32>>& a) {
  using Mint= ModInt<998244353>;
  Matrix<Mint> A(n, n);
  for(int i= 0; i < n; ++i)
   for(int j= 0; j < n; ++j) A[i][j]= Mint(a[i][j]);
  return LU_Decomposition(A).det().val();
 }
};
