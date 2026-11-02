#pragma once
// zmod_rank_blk8 の行列を huge page の領域に置き、要の行を組の 8 本分だけ持つ版 (_shared/linalg/lazy_elim_v.hpp の ElimH)。
// 行が長くなる向き (行の数が min(N, M)) に置く。選ぶ要は zmod_rank_blk8 と同じ。法は 998244353。
#include "../common.hpp"
#include "neo/algebra/ZMod.hpp"
#include "_shared/linalg/lazy_elim_v.hpp"
inline int run(int n, int m, const vector<vector<u32>>& a) {
 using Z= ZMod<MOD>;
 const bool tr= n > m;
 const int rows= tr ? m : n, cols= tr ? n : m, w= (cols + 3) & ~3;
 lazy_elim_v::ElimH<Z> e(rows, w);
 if(tr) {
  for(int j= 0; j < m; ++j) {
   u64* r= e.row(j);
   for(int i= 0; i < n; ++i) r[i]= a[i][j];
   for(int i= n; i < w; ++i) r[i]= 0;
  }
 } else
  for(int i= 0; i < n; ++i) {
   u64* r= e.row(i);
   for(int j= 0; j < m; ++j) r[j]= a[i][j];
   for(int j= m; j < w; ++j) r[j]= 0;
  }
 fill(e.row(rows), e.row(rows) + 8, 0ull);
 e.run(cols, false);
 return e.prow.size();
}
