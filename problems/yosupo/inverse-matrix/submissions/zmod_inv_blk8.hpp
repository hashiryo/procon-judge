#pragma once
// 要を 8 本ずつまとめて掃き出す核 (_shared/linalg/lazy_elim.hpp) で逆行列を求める。法は 998244353。
// [A | I] を前進消去すると要の行が [T | Y] (T は上三角、Y A = T) になるので、T X = Y を後退代入で解くと X = A^{-1}。
#include "../common.hpp"
#include "neo/algebra/ZMod.hpp"
#include "_shared/linalg/lazy_elim.hpp"
inline InverseResult run(int n, const vector<vector<u32>>& a) {
 using Z= ZMod<MOD>;
 lazy_elim::Elim<Z> e(n, (2 * n + 3) & ~3);
 for(int i= 0; i < n; ++i) copy(a[i].begin(), a[i].end(), e.row(i)), e.row(i)[n + i]= 1;
 if(!e.run(n, true)) return {false, {}};
 vector<int> cols(n);
 iota(cols.begin(), cols.end(), n);
 int wx;
 const auto X= lazy_elim::tri_solve(e, cols, wx);
 InverseResult res{true, vector<vector<u32>>(n, vector<u32>(n))};
 for(int t= 0; t < n; ++t)
  for(int j= 0; j < n; ++j) res.mat[t][j]= u32(X[(size_t)t * wx + j]);
 return res;
}
