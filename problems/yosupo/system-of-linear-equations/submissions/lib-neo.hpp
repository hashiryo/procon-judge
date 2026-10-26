#pragma once
// NeoLibrary の solve_with_kernel (neo/linalg/solve.hpp)。この問題で書き比べた zmod_sle_blk8t を写したもので、解と基底を
// ZMod の vector と Matrix で返す分と、それをハーネスの形に直す分が加わる。
#include "../common.hpp"
#include "neo/linalg/solve.hpp"
inline SolveResult run(int n, int m, const vector<vector<u32>>& A, const vector<u32>& b) {
 using Z= ZMod<MOD>;
 Matrix<Z> M(n, m);
 vector<Z> bz(n);
 for(int i= 0; i < n; ++i) {
  for(int j= 0; j < m; ++j) M[i][j]= Z::raw(A[i][j]);
  bz[i]= Z::raw(b[i]);
 }
 const auto r= solve_with_kernel(M, bz);
 if(!r) return {false, {}, {}};
 const auto& [x, K]= *r;
 SolveResult res{true, vector<u32>(m), vector<vector<u32>>(K.h, vector<u32>(m))};
 for(int j= 0; j < m; ++j) res.sol[j]= x[j].val();
 for(int k= 0; k < K.h; ++k)
  for(int j= 0; j < m; ++j) res.basis[k][j]= K[k][j].val();
 return res;
}
