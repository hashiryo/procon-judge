#pragma once
// NeoLibrary の MinCostFlow (neo/combinatorial_optimization/MinCostFlow.hpp) の既定の中身 (cost scaling) で解くもの。
#include "neo/combinatorial_optimization/MinCostFlow.hpp"
struct Solver {
 int m;
 MinCostFlow<long long, long long> g;
 bool ok= false;
 vector<i64> pot, fl;
 Solver(int n, const vector<i64>& b, const vector<array<i64, 5>>& edges): m((int)edges.size()), g(n) {
  for(int v= 0; v < n; ++v) g.add_supply(v, b[v]);
  for(auto& e: edges) g.add_edge((int)e[0], (int)e[1], e[2], e[3], e[4]);
 }
 void run() {
  if(!(ok= g.solve())) return;
  auto p= g.potential();
  pot.assign(p.begin(), p.end()), fl.resize(m);
  for(int i= 0; i < m; ++i) fl[i]= g.get_edge(i).flow;
 }
 bool feasible() const { return ok; }
 __int128 cost() const { return g.cost(); }
 const vector<i64>& potential() const { return pot; }
 const vector<i64>& flow() const { return fl; }
};
