#pragma once
// 余因子行列 adj(A) を、乱択の行と列で 1 回り大きくした B の逆行列から求める (cp-algo の提出 285779 と同じ考え方)。法は 998244353。
// B = [[A, u], [v^T, w]] が正則なら、Jacobi の小行列式の公式で adj(A)[i][j] = det(B) (Bi[n][n] Bi[i][j] - Bi[i][n] Bi[n][j])
// (Bi = B^{-1})。A の階数が n - 1 以上なら B はほぼ確実に正則で、n - 2 以下なら B も正則でなく adj(A) = 0。
// 逆行列と行列式は、要を 8 本ずつまとめて掃き出す核 (_shared/linalg/lazy_elim.hpp) で [B | I] を前進消去し、後退代入はブロックで解く。
#include <chrono>
#include "../common.hpp"
#include "neo/algebra/ZMod.hpp"
#include "_shared/linalg/lazy_elim.hpp"
inline vector<vector<u32>> run(int n, const vector<vector<u32>>& a) {
 using Z= ZMod<MOD>;
 const int m= n + 1;
 vector<vector<u32>> res(n, vector<u32>(n));
 mt19937_64 rng(chrono::steady_clock::now().time_since_epoch().count());
 lazy_elim::Elim<Z> e(m, (2 * m + 3) & ~3);
 for(int i= 0; i < m; ++i) {
  u64* row= e.row(i);
  if(i < n) copy(a[i].begin(), a[i].end(), row), row[n]= rng() % MOD;
  else
   for(int j= 0; j <= n; ++j) row[j]= rng() % MOD;
  row[m + i]= 1;
 }
 if(!e.run(m, true)) return res;
 vector<int> cols(m);
 iota(cols.begin(), cols.end(), m);
 int wx;
 const auto X= lazy_elim::tri_solve_blk(e, cols, wx);
 Z d= Z::raw(1);
 for(u32 p : e.pval) d*= Z::raw(p);
 vector<char> seen(m);
 int cyc= 0;
 for(int i= 0; i < m; ++i)
  if(!seen[i]) {
   ++cyc;
   for(int j= i; !seen[j]; j= e.prow[j]) seen[j]= 1;
  }
 if((m - cyc) & 1) d= -d;
 auto bi= [&](int i, int j) { return Z::raw(u32(X[(size_t)i * wx + j])); };
 const Z dn= d * bi(n, n);
 vector<Z> col(n);  // d Bi[i][n]
 for(int i= 0; i < n; ++i) col[i]= d * bi(i, n);
 for(int i= 0; i < n; ++i)
  for(int j= 0; j < n; ++j) res[i][j]= (dn * bi(i, j) - col[i] * bi(n, j)).val();
 return res;
}
