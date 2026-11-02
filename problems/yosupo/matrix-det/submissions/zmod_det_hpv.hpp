#pragma once
// zmod_det_hp の、要を探す列、要の行、行ごとの掛ける数を AVX2 で 4 個ずつ求める版 (_shared/linalg/lazy_elim_v.hpp の ElimV)。
// 選ぶ要は zmod_det_blk8 と同じ。要の値は 2^{-32} 倍で返るので、最後に 2^{32n} を掛ける。法は 998244353。
#include "../common.hpp"
#include "neo/algebra/ZMod.hpp"
#include "_shared/linalg/lazy_elim_v.hpp"
inline u32 run(int n, const vector<vector<u32>>& a) {
 using Z= ZMod<MOD>;
 const int w= (n + 3) & ~3;
 lazy_elim_v::ElimV<Z> e(n, w);
 for(int i= 0; i < n; ++i) {
  u64* r= e.row(i);
  for(int j= 0; j < n; ++j) r[j]= a[i][j];
  for(int j= n; j < w; ++j) r[j]= 0;
 }
 fill(e.row(n), e.row(n) + 8, 0ull);
 if(!e.run(n, true)) return 0;
 Z d= Z((u64(1) << 32) % MOD).pow(n);
 for(u32 p : e.pvalm) d*= Z::raw(p);
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
