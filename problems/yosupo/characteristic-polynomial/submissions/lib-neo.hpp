#pragma once
// NeoLibrary の characteristic_polynomial (neo/linalg/characteristic_polynomial.hpp)。この問題で書き比べた zmod_hess32 を、
// 法を定数にしない形で写したもの。
#include "../common.hpp"
#include "neo/linalg/characteristic_polynomial.hpp"
inline vector<u32> run(int n, const vector<vector<u32>>& M) {
 using Z= ZMod<MOD>;
 Matrix<Z> A(n, n);
 for(int i= 0; i < n; ++i)
  for(int j= 0; j < n; ++j) A[i][j]= Z::raw(M[i][j]);
 const auto p= characteristic_polynomial(std::move(A));
 vector<u32> res(n + 1);
 for(int i= 0; i <= n; ++i) res[i]= p[i].val();
 return res;
}
