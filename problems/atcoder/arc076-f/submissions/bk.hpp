#pragma once
// Boykov-Kolmogorov (s と t から探索木を育て、流して切れた木の辺の先を付け直して使い続ける) で解く力ずく。網は lib-neo と
// 同じで、椅子の上に左から累積する前の鎖と、右から累積する後ろの鎖を置く。核は _bk.hpp にある。push-relabel と Dinic が
// 遅い右側だけの鎖 (seed 8) でも、探索木を使い回すので増加路を探し直さずに済む。
#include "_bk.hpp"
struct Solver {
 int n, m;
 const vector<array<int, 2>>& lr;
 int ans= 0;
 Solver(int n, int m, const vector<array<int, 2>>& lr): n(n), m(m), lr(lr) {}
 void run() {
  // 頂点: 人 0..n-1、前の鎖 P + k、後ろの鎖 S + k、椅子 C + k (k = 1..m)、s、t
  const int P= n - 1, S= n + m - 1, C= n + 2 * m - 1, s= n + 3 * m, t= s + 1;
  vector<array<int, 4>> e;
  e.reserve(3 * (size_t)n + 5 * (size_t)m);
  for(int i= 0; i < n; ++i) {
   e.push_back({s, i, 1, 0});
   if(lr[i][0] >= 1) e.push_back({i, P + lr[i][0], 1, 0});
   if(lr[i][1] <= m) e.push_back({i, S + lr[i][1], 1, 0});
  }
  for(int k= 1; k <= m; ++k) {
   if(k > 1) e.push_back({P + k, P + k - 1, n, 0});
   if(k < m) e.push_back({S + k, S + k + 1, n, 0});
   e.push_back({P + k, C + k, 1, 0}), e.push_back({S + k, C + k, 1, 0}), e.push_back({C + k, t, 1, 0});
  }
  ans= n - (int)mf_bk::max_flow(t + 1, s, t, e);
 }
 int answer() const { return ans; }
};
