#pragma once
// NeoLibrary の MaxFlow (neo/flow/MaxFlow.hpp、容量は int) で最大重み閉包の最小カットを求める。s → 仕事に収入、
// 仕事 → 機械に借りる費用、機械 → t に買う費用の容量を張り、収入の和から最大流を引く。頂点 2402、辺は最大 1.44 × 10^6 本。
#include "neo/flow/MaxFlow.hpp"
struct Solver {
 int n, m;
 const vector<int>&x, &off;
 const vector<array<int, 2>>& proc;
 const vector<int>& y;
 long long ans= 0;
 Solver(int n, int m, const vector<int>& x, const vector<int>& off, const vector<array<int, 2>>& proc, const vector<int>& y): n(n), m(m), x(x), off(off), proc(proc), y(y) {}
 void run() {
  const int s= n + m, t= s + 1;
  MaxFlow<int> g(t + 1);
  long long sum= 0;
  for(int i= 0; i < n; ++i) {
   g.add_edge(s, i, x[i]), sum+= x[i];
   for(int k= off[i]; k < off[i + 1]; ++k) g.add_edge(i, n + proc[k][0], proc[k][1]);
  }
  for(int j= 0; j < m; ++j) g.add_edge(n + j, t, y[j]);
  ans= sum - g.flow(s, t);
 }
 long long answer() const { return ans; }
};
