#pragma once
// NeoLibrary の hafnian (neo/linalg/hafnian.hpp)。この問題で書き比べた zmod_layer_h の法を実行時の奇数に広げたもの。
#include "../common.hpp"
#include "neo/linalg/hafnian.hpp"
inline u32 run(int N, const vector<vector<u32>>& a) {
 using Z= ZMod<MOD>;
 Matrix<Z> A(N, N);
 for(int i= 0; i < N; ++i)
  for(int j= 0; j < N; ++j) A[i][j]= Z::raw(a[i][j]);
 return hafnian(A).val();
}
