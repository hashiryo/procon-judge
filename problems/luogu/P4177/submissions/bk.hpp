#pragma once
// Boykov-Kolmogorov (s と t から探索木を育て、流して切れた木の辺の先を付け直して使い続ける) で最大重み閉包の最小カットを
// 求める。網は lib-neo と同じ。核は _bk.hpp にある。
#include "_bk.hpp"
struct Solver {
 int n, m;
 const vector<int>&x, &off;
 const vector<array<int, 2>>& proc;
 const vector<int>& y;
 long long ans= 0;
 Solver(int n, int m, const vector<int>& x, const vector<int>& off, const vector<array<int, 2>>& proc, const vector<int>& y): n(n), m(m), x(x), off(off), proc(proc), y(y) {}
 void run() {
  const int s= n + m, t= s + 1;
  vector<array<int, 4>> e;
  e.reserve(n + proc.size() + m);
  long long sum= 0;
  for(int i= 0; i < n; ++i) {
   e.push_back({s, i, x[i], 0}), sum+= x[i];
   for(int k= off[i]; k < off[i + 1]; ++k) e.push_back({i, n + proc[k][0], proc[k][1], 0});
  }
  for(int j= 0; j < m; ++j) e.push_back({n + j, t, y[j], 0});
  ans= sum - mf_bk::max_flow(t + 1, s, t, e);
 }
 long long answer() const { return ans; }
};
