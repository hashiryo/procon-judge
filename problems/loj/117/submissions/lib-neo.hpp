#pragma once
// NeoLibrary の MaxFlowLowerBound (neo/combinatorial_optimization/MaxFlowLowerBound.hpp) の min_flow。
#include "neo/combinatorial_optimization/MaxFlowLowerBound.hpp"
struct Solver {
 MaxFlowLowerBound<long long> g;
 int s, t;
 long long ans= -1;
 Solver(int n, int s, int t, const vector<array<int, 4>>& edges): g(n), s(s), t(t) {
  for(auto& e: edges) g.add_edge(e[0], e[1], e[2], e[3]);
 }
 void run() { ans= g.min_flow(s, t).value_or(-1); }
 long long answer() const { return ans; }
};
