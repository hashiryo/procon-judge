#pragma once
// abc246-h の愚直解 (提出ではない)。更新のたびに、'?' の置き換え方と部分列の選び方を全部並べて、得られる列を集合に
// 入れて数える。行列も DP も使わないので、提出とは別の考え方になる。N ≤ 8 程度の小さい入力でだけ使う
// (pj testdata crosscheck --brute brute.hpp)。
#include "common.hpp"
struct Solver {
 string s;
 explicit Solver(const string& s): s(s) {}
 u32 update(int x, char c) {
  s[x]= c;
  const int n= s.size();
  vector<int> qs;
  for(int i= 0; i < n; ++i)
   if(s[i] == '?') qs.push_back(i);
  set<string> seen;
  for(int r= 0; r < (1 << qs.size()); ++r) {
   string u= s;
   for(size_t k= 0; k < qs.size(); ++k) u[qs[k]]= (r >> k & 1) ? '1' : '0';
   for(int m= 1; m < (1 << n); ++m) {
    string w;
    for(int i= 0; i < n; ++i)
     if(m >> i & 1) w+= u[i];
    seen.insert(w);
   }
  }
  return u32(seen.size() % MOD);
 }
};
