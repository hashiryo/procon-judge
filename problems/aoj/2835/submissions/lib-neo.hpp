#pragma once
// NeoLibrary の MaxFlow (neo/flow/MaxFlow.hpp) で書いたもの。解き方は common.hpp と同じで、容量 1 の辺を 1 本ずつ change_cap で
// 塞ぎ、流量が減るものがあれば答えを 1 減らす。辺は両向き。
#include "neo/flow/MaxFlow.hpp"
struct Solver {
 int n;
 const vector<array<i64, 3>>& edges;
 i64 ans= 0;
 Solver(int n, const vector<array<i64, 3>>& edges): n(n), edges(edges) {}
 void run() {
  MaxFlow<i64> g(n);
  for(auto& e: edges) g.add_edge((int)e[0], (int)e[1], e[2], e[2]);
  ans= g.flow(0, n - 1);
  for(size_t i= 0; i < edges.size(); ++i)
   if(edges[i][2] == 1) {
    if(g.change_cap((int)i, 0, 0, 0, n - 1)) {
     --ans;
     break;
    }
    g.change_cap((int)i, 1, 1, 0, n - 1);
   }
  if(ans > 10000) ans= -1;
 }
 i64 answer() const { return ans; }
};
