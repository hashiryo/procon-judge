#pragma once
// NeoLibrary の MaxFlowLowerBound (neo/combinatorial_optimization/MaxFlowLowerBound.hpp) の solve で流れを求め、edges で
// 辺ごとの流量を読む。
#include "neo/combinatorial_optimization/MaxFlowLowerBound.hpp"
struct Solver {
 MaxFlowLowerBound<long long> g;
 bool ok= false;
 vector<i64> f;
 Solver(int n, const vector<array<int, 4>>& edges): g(n) {
  for(auto& e: edges) g.add_edge(e[0], e[1], e[2], e[3]);
 }
 void run() {
  if(!(ok= g.solve())) return;
  const auto es= g.edges();
  f.resize(es.size());
  for(size_t i= 0; i < es.size(); ++i) f[i]= es[i].flow;
 }
 bool feasible() const { return ok; }
 const vector<i64>& flows() const { return f; }
};
