#pragma once
// 今の Library の MaxFlow (Dinic、容量は int) で解く力ずく。網は lib-neo と同じで、格子の無向の道を両向きの辺 (bidir) で
// 足し、左上から右下への最大流を求める。最小カットが真ん中の帯にあるケース (seed 2、3) と蛇行する道のケース (seed 9) は、
// 段の数が増えて、手元の M2 で 5 秒を超える。
#include "mylib/optimization/MaxFlow.hpp"
struct Solver {
 int n, m;
 const vector<int>&h, &v, &d;
 long long ans= 0;
 Solver(int n, int m, const vector<int>& h, const vector<int>& v, const vector<int>& d): n(n), m(m), h(h), v(v), d(d) {}
 void run() {
  MaxFlow<Dinic<int>> g(n * m);
  for(int i= 0; i < n; ++i)
   for(int j= 0; j < m; ++j) {
    const int x= i * m + j;
    if(j + 1 < m) g.add_edge(x, x + 1, h[i * (m - 1) + j], true);
    if(i + 1 < n) g.add_edge(x, x + m, v[i * m + j], true);
    if(i + 1 < n && j + 1 < m) g.add_edge(x, x + m + 1, d[i * (m - 1) + j], true);
   }
  ans= g.maxflow(0, n * m - 1);
 }
 long long answer() const { return ans; }
};
