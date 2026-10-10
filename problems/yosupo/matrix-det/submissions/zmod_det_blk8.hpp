#pragma once
// 要を 8 本ずつまとめて掃き出す核 (_shared/linalg/lazy_elim.hpp) で行列式を求める。法は 998244353。
#include "../common.hpp"
#include "neo/algebra/ZMod.hpp"
#include "_shared/linalg/lazy_elim.hpp"
inline u32 run(int n, const vector<vector<u32>>& a) {
 using Z= ZMod<MOD>;
 lazy_elim::Elim<Z> e(n, (n + 3) & ~3);
 for(int i= 0; i < n; ++i) copy(a[i].begin(), a[i].end(), e.row(i));
 if(!e.run(n, true)) return 0;
 Z d= Z::raw(1);
 for(u32 p : e.pval) d*= Z::raw(p);
 // 要にした行の並び (t 番目の要が行 prow[t]) の置換の符号
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
