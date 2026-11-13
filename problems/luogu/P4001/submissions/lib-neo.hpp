#pragma once
// NeoLibrary の MaxFlow (neo/combinatorial_optimization/MaxFlow.hpp、容量は int) で解く力ずく。格子の無向の道を、両向きの容量が等しい辺として
// 足し、左上から右下への最大流を求める。頂点 10^6、辺 3 × 10^6 で、MaxFlow の弧は 12 byte。
#include "neo/combinatorial_optimization/MaxFlow.hpp"
struct Solver {
 int n, m;
 const vector<int>&h, &v, &d;
 long long ans= 0;
 Solver(int n, int m, const vector<int>& h, const vector<int>& v, const vector<int>& d): n(n), m(m), h(h), v(v), d(d) {}
 void run() {
  MaxFlow<int> g(n * m);
  for(int i= 0; i < n; ++i)
   for(int j= 0; j < m; ++j) {
    const int x= i * m + j;
    if(j + 1 < m) g.add_edge(x, x + 1, h[i * (m - 1) + j], h[i * (m - 1) + j]);
    if(i + 1 < n) g.add_edge(x, x + m, v[i * m + j], v[i * m + j]);
    if(i + 1 < n && j + 1 < m) g.add_edge(x, x + m + 1, d[i * (m - 1) + j], d[i * (m - 1) + j]);
   }
  ans= g.flow(0, n * m - 1);
 }
 long long answer() const { return ans; }
};
