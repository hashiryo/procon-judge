#pragma once
// NeoLibrary の pfaffian (neo/linalg/pfaffian.hpp)。この問題で書き比べた zmod_pf_blk8 を写したもの。奇数番目の列に要がない
// とき 0 を返さずに進める分が違う (zmod_pf_blk8 と naive は、対角に 2 × 2 の塊を並べた行列で 0 を返す)。
#include "../common.hpp"
#include "neo/linalg/pfaffian.hpp"
inline u32 run(int N, const vector<vector<u32>>& M_in) {
 using Z= ZMod<MOD>;
 const int n= 2 * N;
 Matrix<Z> A(n, n);
 for(int i= 0; i < n; ++i)
  for(int j= 0; j < n; ++j) A[i][j]= Z::raw(M_in[i][j]);
 return pfaffian(std::move(A)).val();
}
