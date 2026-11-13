#pragma once
// 確かめ用。lib-neo と同じく Matrix<Z> に写してから、作業の行列を 0 で埋めない HugeBuf に替えた NeoLibrary の zmod_v の写し
// (_neo_pf_hb.hpp) を呼ぶ。lib-neo との差は、作業の行列 (n = 1000 で 8 MB) を値の初期化で 0 で埋める分。
#include "_neo_pf_hb.hpp"
inline u32 run(int N, const vector<vector<u32>>& M_in) {
 using Z= ZMod<MOD>;
 const int n= 2 * N;
 Matrix<Z> A(n, n);
 for(int i= 0; i < n; ++i)
  for(int j= 0; j < n; ++j) A[i][j]= Z::raw(M_in[i][j]);
 return neo_pf_hb::zmod_v<Z>(n, [&](int i, int j) { return A[i][j].val(); }).val();
}
