#pragma once
// Boykov-Kolmogorov (s と t から探索木を育て、流して切れた木の辺の先を付け直して使い続ける) で解く力ずく。網は lib-neo と
// 同じで、格子の無向の道を両向きの容量が等しい辺にする。核は _bk.hpp にある。画像の領域分割のような格子で速いことで
// 知られる方法で、辺の並びを 1 度写すので、メモリは lib-neo より多い。蛇行する道のケース (seed 9) は、手元の M2 で 5 秒を
// 超える。
#include "_bk.hpp"
struct Solver {
 int n, m;
 const vector<int>&h, &v, &d;
 long long ans= 0;
 Solver(int n, int m, const vector<int>& h, const vector<int>& v, const vector<int>& d): n(n), m(m), h(h), v(v), d(d) {}
 void run() {
  vector<array<int, 4>> e;
  e.reserve(h.size() + v.size() + d.size());
  for(int i= 0; i < n; ++i)
   for(int j= 0; j < m; ++j) {
    const int x= i * m + j;
    if(j + 1 < m) e.push_back({x, x + 1, h[i * (m - 1) + j], h[i * (m - 1) + j]});
    if(i + 1 < n) e.push_back({x, x + m, v[i * m + j], v[i * m + j]});
    if(i + 1 < n && j + 1 < m) e.push_back({x, x + m + 1, d[i * (m - 1) + j], d[i * (m - 1) + j]});
   }
  ans= mf_bk::max_flow(n * m, 0, n * m - 1, e);
 }
 long long answer() const { return ans; }
};
