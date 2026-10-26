#pragma once
// NeoLibrary の inverse (neo/linalg/inverse.hpp)。この問題で書き比べた zmod_inv_relabel を写したもの。
#include "../common.hpp"
#include "neo/linalg/inverse.hpp"
inline InverseResult run(int n, const vector<vector<u32>>& a) {
 using Z= ZMod<MOD>;
 Matrix<Z> A(n, n);
 for(int i= 0; i < n; ++i)
  for(int j= 0; j < n; ++j) A[i][j]= Z::raw(a[i][j]);
 const auto X= inverse(std::move(A));
 if(!X) return {false, {}};
 InverseResult res{true, vector<vector<u32>>(n, vector<u32>(n))};
 for(int i= 0; i < n; ++i)
  for(int j= 0; j < n; ++j) res.mat[i][j]= (*X)[i][j].val();
 return res;
}
