#pragma once
// cf-1609-e の愚直解 (提出ではない)。更新のたびに、3^n 通りの列を全部試し、"abc" を部分列に持たないもののうち、
// 今の列と違う文字の数が最小のものを答える。行列も DP も使わないので、提出とは別の考え方になる。n ≤ 7 程度の
// 小さい入力でだけ使う (pj testdata crosscheck --brute brute.hpp)。
#include "common.hpp"
struct Solver {
 string s;
 explicit Solver(const string& s): s(s) {}
 int update(int x, char c) {
  s[x]= c;
  const int n= s.size();
  int total= 1;
  for(int i= 0; i < n; ++i) total*= 3;
  int best= n;
  string t(n, 'a');
  for(int code= 0; code < total; ++code) {
   int v= code, diff= 0;
   for(int i= 0; i < n; ++i) t[i]= "abc"[v % 3], v/= 3, diff+= t[i] != s[i];
   if(diff >= best) continue;
   int j= 0;
   for(int i= 0; i < n && j < 3; ++i)
    if(t[i] == "abc"[j]) ++j;
   if(j < 3) best= diff;
  }
  return best;
 }
};
