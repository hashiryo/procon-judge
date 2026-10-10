#pragma once
// 要を 8 本ずつまとめて掃き出す核 (_shared/linalg/lazy_elim.hpp) で階数を求める。法は 998244353。
// 行が長くなる向き (行の数が min(N, M)) に置く。
#include "../common.hpp"
#include "neo/algebra/ZMod.hpp"
#include "_shared/linalg/lazy_elim.hpp"
inline int run(int n, int m, const vector<vector<u32>>& a) {
 using Z= ZMod<MOD>;
 const bool tr= n > m;
 const int rows= tr ? m : n, cols= tr ? n : m;
 lazy_elim::Elim<Z> e(rows, (cols + 3) & ~3);
 if(tr) {
  for(int i= 0; i < n; ++i)
   for(int j= 0; j < m; ++j) e.row(j)[i]= a[i][j];
 } else
  for(int i= 0; i < n; ++i) copy(a[i].begin(), a[i].end(), e.row(i));
 e.run(cols, false);
 return e.prow.size();
}
