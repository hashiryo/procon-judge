#pragma once
// NeoLibrary の MaxFlowLowerBound (neo/combinatorial_optimization/MaxFlowLowerBound.hpp) で、common.hpp の FlowSolver と
// 同じ手順を踏む。幅 d を 0 から増やし、同じ幅の中では下限 l が大きい方から試して、max_flow が辺の数 m になるかを見る。
// 頂点と辺の並びも FlowSolver と同じにする。容量は FlowSolver と同じく 64 bit。
#include "pj.hpp"
#include "neo/combinatorial_optimization/MaxFlowLowerBound.hpp"
struct Solver {
 int n;
 const vector<array<int, 2>>& edges;
 array<i64, 2> ans{0, 0};
 Solver(int n, const vector<array<int, 2>>& edges): n(n), edges(edges) {}
 void run() {
  const int m= (int)edges.size();
  for(int d= 0; d <= n; ++d)
   for(int l= n - d; l >= 0; --l) {
    const int r= l + d;
    MaxFlowLowerBound<long long> g(2 + m + n);
    const int s= 0, t= 1, e= 2, w= 2 + m;
    for(int i= 0; i < m; ++i) {
     g.add_edge(s, e + i, 0, 1);
     g.add_edge(e + i, w + edges[i][0], 0, 1);
     g.add_edge(e + i, w + edges[i][1], 0, 1);
    }
    // 各頂点が受け持つ本数を [l, r] に収める。
    for(int i= 0; i < n; ++i) g.add_edge(w + i, t, l, r);
    if(g.max_flow(s, t) == m) {
     ans= {l, r};
     return;
    }
   }
 }
 array<i64, 2> answer() const { return ans; }
};
