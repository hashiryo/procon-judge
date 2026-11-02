#pragma once
// zmod_det_blk8 の行列を huge page の領域に置き、要の行を組の 8 本分だけ持つ版 (_shared/linalg/lazy_elim_v.hpp の ElimH)。
// 手順と選ぶ要は zmod_det_blk8 と同じ。法は 998244353。
#include "../common.hpp"
#include "neo/algebra/ZMod.hpp"
#include "_shared/linalg/lazy_elim_v.hpp"
inline u32 run(int n, const vector<vector<u32>>& a) {
 using Z= ZMod<MOD>;
 const int w= (n + 3) & ~3;
 lazy_elim_v::ElimH<Z> e(n, w);
 for(int i= 0; i < n; ++i) {
  u64* r= e.row(i);
  for(int j= 0; j < n; ++j) r[j]= a[i][j];
  for(int j= n; j < w; ++j) r[j]= 0;
 }
 fill(e.row(n), e.row(n) + 8, 0ull);
 if(!e.run(n, true)) return 0;
 Z d= Z::raw(1);
 for(u32 p : e.pval) d*= Z::raw(p);
 vector<char> seen(n);
 int cyc= 0;
 for(int i= 0; i < n; ++i)
  if(!seen[i]) {
   ++cyc;
   for(int j= i; !seen[j]; j= e.prow[j]) seen[j]= 1;
  }
 if((n - cyc) & 1) d= -d;
 return d.val();
}
