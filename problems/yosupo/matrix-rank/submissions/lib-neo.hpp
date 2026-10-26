#pragma once
// NeoLibrary の rank (neo/linalg/rank.hpp)。この問題で書き比べた zmod_rank_blk8 を写したもの。using namespace std; のもとでは
// std::rank と名前がぶつかるので ::rank と書く。
#include "../common.hpp"
#include "neo/linalg/rank.hpp"
inline int run(int n, int m, const vector<vector<u32>>& a) {
 using Z= ZMod<MOD>;
 Matrix<Z> A(n, m);
 for(int i= 0; i < n; ++i)
  for(int j= 0; j < m; ++j) A[i][j]= Z::raw(a[i][j]);
 return ::rank(std::move(A));
}
