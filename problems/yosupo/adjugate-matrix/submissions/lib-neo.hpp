#pragma once
// NeoLibrary の adjugate (neo/linalg/adjugate.hpp)。zmod_adj_blk8t の乱択 (1 回り大きくした行列の逆行列) はやめ、階数で分ける。
// 正則なら det(A) A^{-1}、階数 n - 1 なら核と左の核の積の定数倍で、前進消去と後退代入は inverse の核 (zmod_inv_relabel の写し)。
#include "../common.hpp"
#include "neo/linalg/adjugate.hpp"
inline vector<vector<u32>> run(int n, const vector<vector<u32>>& a) {
 using Z= ZMod<MOD>;
 Matrix<Z> A(n, n);
 for(int i= 0; i < n; ++i)
  for(int j= 0; j < n; ++j) A[i][j]= Z::raw(a[i][j]);
 const Matrix<Z> B= adjugate(A);
 vector<vector<u32>> res(n, vector<u32>(n));
 for(int i= 0; i < n; ++i)
  for(int j= 0; j < n; ++j) res[i][j]= B[i][j].val();
 return res;
}
