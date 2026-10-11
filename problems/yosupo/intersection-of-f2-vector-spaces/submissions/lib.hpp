#pragma once
// Library の LU_Decomposition<bool> の kernel で求める。u_i と v_j を列に並べた 30 × (n + m) の行列の核 (a, b) は
// sum a_i u_i = sum b_j v_j を満たすので、w = sum b_j v_j が共通部分の基底になる (U と V はそれぞれ線形独立)。
#include "../common.hpp"
#include "mylib/algebra/LU_Decomposition.hpp"
inline int run(const u32* u, int n, const u32* v, int m, u32* w) {
 if(n == 0 || m == 0) return 0;
 Matrix<bool> A(30, n + m);
 for(int b= 0; b < 30; ++b) {
  for(int i= 0; i < n; ++i) A[b][i]= u[i] >> b & 1;
  for(int j= 0; j < m; ++j) A[b][n + j]= v[j] >> b & 1;
 }
 const auto ker= LU_Decomposition<bool>(A).kernel();
 int k= 0;
 for(const auto& x : ker) {
  u32 s= 0;
  for(int j= 0; j < m; ++j)
   if(x[n + j]) s^= v[j];
  w[k++]= s;
 }
 return k;
}
