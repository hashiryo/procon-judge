#pragma once
// NeoLibrary の MaxFlow (neo/flow/MaxFlow.hpp、容量は int) で解く力ずく。椅子の上に左から累積する前の鎖と、右から累積
// する後ろの鎖を置き、s → 人 (容量 1)、人 → 前の鎖の L_i と後ろの鎖の R_i (容量 1)、前の鎖 k → k - 1 と後ろの鎖
// k → k + 1 (容量 N)、どちらの鎖も k → 椅子 k (容量 1)、椅子 → t (容量 1) と張る。答えは N から最大流を引いたもの。
// 人が椅子より多いと、t に届かない余りが鎖を行き来する (MaxFlow は途中で増加路へ移る)。右側だけの鎖 (seed 8) は、
// 増加路が鎖を長く往復するので、push-relabel のまま 5 秒を超える (NeoLibrary 37f7c53 の時点)。
#include "neo/flow/MaxFlow.hpp"
struct Solver {
 int n, m;
 const vector<array<int, 2>>& lr;
 int ans= 0;
 Solver(int n, int m, const vector<array<int, 2>>& lr): n(n), m(m), lr(lr) {}
 void run() {
  // 頂点: 人 0..n-1、前の鎖 P + k、後ろの鎖 S + k、椅子 C + k (k = 1..m)、s、t
  const int P= n - 1, S= n + m - 1, C= n + 2 * m - 1, s= n + 3 * m, t= s + 1;
  MaxFlow<int> g(t + 1);
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
  ans= n - g.flow(s, t);
 }
 int answer() const { return ans; }
};
