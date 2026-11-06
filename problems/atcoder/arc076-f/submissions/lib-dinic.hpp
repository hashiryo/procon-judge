#pragma once
// 今の Library の MaxFlow (Dinic、容量は int) で解く力ずく。網は lib-neo と同じで、椅子の上に左から累積する前の鎖と、
// 右から累積する後ろの鎖を置く。椅子が埋まるにつれて増加路が長くなり、段の数が増える。
#include "mylib/optimization/MaxFlow.hpp"
struct Solver {
 int n, m;
 const vector<array<int, 2>>& lr;
 int ans= 0;
 Solver(int n, int m, const vector<array<int, 2>>& lr): n(n), m(m), lr(lr) {}
 void run() {
  // 頂点: 人 0..n-1、前の鎖 P + k、後ろの鎖 S + k、椅子 C + k (k = 1..m)、s、t
  const int P= n - 1, S= n + m - 1, C= n + 2 * m - 1, s= n + 3 * m, t= s + 1;
  MaxFlow<Dinic<int>> g(t + 1);
  for(int i= 0; i < n; ++i) {
   g.add_edge(s, i, 1);
   if(lr[i][0] >= 1) g.add_edge(i, P + lr[i][0], 1);
   if(lr[i][1] <= m) g.add_edge(i, S + lr[i][1], 1);
  }
  for(int k= 1; k <= m; ++k) {
   if(k > 1) g.add_edge(P + k, P + k - 1, n);
   if(k < m) g.add_edge(S + k, S + k + 1, n);
   g.add_edge(P + k, C + k, 1), g.add_edge(S + k, C + k, 1), g.add_edge(C + k, t, 1);
  }
  ans= n - g.maxflow(s, t);
 }
 int answer() const { return ans; }
};
