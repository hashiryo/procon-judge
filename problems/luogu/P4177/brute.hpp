#pragma once
// 愚直解 (提出ではない)。選ぶ仕事の集合を全部試し、機械ごとに、買う費用と、選んだ仕事が借りる費用の和の小さい方を払う。
// 小さい入力 (N ≤ 8) でだけ使う (pj testdata crosscheck --brute brute.hpp)。
#include <algorithm>
#include <array>
#include <vector>
struct Solver {
 int n, m;
 const vector<int>&x, &off;
 const vector<array<int, 2>>& proc;
 const vector<int>& y;
 long long ans= 0;
 Solver(int n, int m, const vector<int>& x, const vector<int>& off, const vector<array<int, 2>>& proc, const vector<int>& y): n(n), m(m), x(x), off(off), proc(proc), y(y) {}
 void run() {
  for(int mask= 0; mask < (1 << n); ++mask) {
   vector<long long> rent(m);
   long long p= 0;
   for(int i= 0; i < n; ++i)
    if(mask >> i & 1) {
     p+= x[i];
     for(int k= off[i]; k < off[i + 1]; ++k) rent[proc[k][0]]+= proc[k][1];
    }
   for(int j= 0; j < m; ++j) p-= min<long long>(y[j], rent[j]);
   ans= max(ans, p);
  }
 }
 long long answer() const { return ans; }
};
