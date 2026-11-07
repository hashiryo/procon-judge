#pragma once
// lib-neo と同じ呼び方で、NeoLibrary の inverse に run_v の差分を当てた写し (_neo_inverse_runv.hpp) を呼ぶ。lib-neo と同じ回で
// 比べて、差分を NeoLibrary に入れるかを決める。決めたら消す。
#include "../common.hpp"
#include "_neo_inverse_runv.hpp"
inline InverseResult run(int n, const vector<vector<u32>>& a) {
 using Z= ZMod<MOD>;
 Matrix<Z> A(n, n);
 for(int i= 0; i < n; ++i)
  for(int j= 0; j < n; ++j) A[i][j]= Z::raw(a[i][j]);
 const auto X= inverse_runv(std::move(A));
 if(!X) return {false, {}};
 InverseResult res{true, vector<vector<u32>>(n, vector<u32>(n))};
 for(int i= 0; i < n; ++i)
  for(int j= 0; j < n; ++j) res.mat[i][j]= (*X)[i][j].val();
 return res;
}
