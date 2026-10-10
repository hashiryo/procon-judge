#pragma once
// zmod_sle_blk8 の後退代入を、下から 8 行ずつのブロックで解く形 (lazy_elim::tri_solve_blk) にしたもの。法は 998244353。
// [A | b] を前進消去し、要の列が作る上三角の行列で、自由な列と b の列をまとめて後退代入する。
#include "../common.hpp"
#include "neo/algebra/ZMod.hpp"
#include "_shared/linalg/lazy_elim.hpp"
inline SolveResult run(int n, int m, const vector<vector<u32>>& A, const vector<u32>& b) {
 using Z= ZMod<MOD>;
 lazy_elim::Elim<Z> e(n, (m + 1 + 3) & ~3);
 for(int i= 0; i < n; ++i) copy(A[i].begin(), A[i].end(), e.row(i)), e.row(i)[m]= b[i];
 e.run(m, false);
 // 残りの行は A の部分が 0 なので、b の値が 0 でなければ解がない
 for(int i : e.rem)
  if(e.value(i, m)) return {false, {}, {}};
 const int r= e.prow.size();
 vector<char> isp(m);
 for(int c : e.pcol) isp[c]= 1;
 vector<int> cols;
 for(int j= 0; j < m; ++j)
  if(!isp[j]) cols.push_back(j);
 const int nf= cols.size();
 cols.push_back(m);
 int wx;
 const auto X= lazy_elim::tri_solve_blk(e, cols, wx);
 SolveResult res{true, vector<u32>(m), vector<vector<u32>>(nf, vector<u32>(m))};
 for(int t= 0; t < r; ++t) res.sol[e.pcol[t]]= u32(X[(size_t)t * wx + nf]);
 for(int k= 0; k < nf; ++k) {
  auto& v= res.basis[k];
  v[cols[k]]= 1;
  for(int t= 0; t < r; ++t) {
   const u32 x= u32(X[(size_t)t * wx + k]);
   v[e.pcol[t]]= x ? MOD - x : 0;
  }
 }
 return res;
}
