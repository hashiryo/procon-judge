#pragma once
// NeoLibrary の det (neo/linalg/det.hpp) を ZMod<0> で。この問題で書き比べた zmod_det_blk8 を写したもの。
#include "../common.hpp"
#include "neo/linalg/det.hpp"
inline u32 run(int n, u32 mod, const vector<vector<u32>>& a) {
 using Z= ZMod<0>;
 Z::set_mod(mod);
 Matrix<Z> A(n, n);
 for(int i= 0; i < n; ++i)
  for(int j= 0; j < n; ++j) A[i][j]= Z::raw(a[i][j]);
 return det(std::move(A)).val();
}
