#pragma once
// zmod_sle_blk8t の前進消去を、要を探す列、要の行、行ごとの掛ける数を AVX2 で 4 個ずつ求め、行列と要の行を huge page に置く
// 形 (_shared/linalg/lazy_elim_vk.hpp の ElimK) にしたもの。要の行と要の値は 2^{-32} 倍で持つが、上三角の行列の各行と右辺が
// 同じ倍率なので、後退代入 (tri_solve_blk) の解は変わらない。法は 998244353。
#include "../common.hpp"
#include "neo/algebra/ZMod.hpp"
#include "_shared/linalg/lazy_elim_vk.hpp"
inline SolveResult run(int n, int m, const vector<vector<u32>>& A, const vector<u32>& b) {
 using Z= ZMod<MOD>;
 const int w= (m + 1 + 3) & ~3;
 lazy_elim_vk::ElimK<Z> e(n, w);
 for(int i= 0; i < n; ++i) {
  u64* r= e.row(i);
  for(int j= 0; j < m; ++j) r[j]= A[i][j];
  r[m]= b[i];
  for(int j= m + 1; j < w; ++j) r[j]= 0;
 }
 fill(e.row(n), e.row(n) + 8, 0ull);
 e.run(m);
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
 const auto X= lazy_elim_vk::tri_solve_blk(e, cols, wx);
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
