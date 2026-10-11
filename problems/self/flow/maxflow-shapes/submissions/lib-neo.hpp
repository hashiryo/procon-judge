#pragma once
// NeoLibrary の MaxFlow (neo/flow/MaxFlow.hpp、容量は i64)。逆向きの容量は add_edge の rev_cap に渡す。flow だけを呼ぶので、
// 余りを s へ戻す段は回らない。
#include "neo/flow/MaxFlow.hpp"
struct Solver {
 MaxFlow<long long> g;
 int s, t;
 long long ans= 0;
 Solver(int n, int s, int t, const vector<array<int, 4>>& edges): g(n), s(s), t(t) {
  for(auto& e: edges) g.add_edge(e[0], e[1], e[2], e[3]);
 }
 void run() { ans= g.flow(s, t); }
 long long answer() const { return ans; }
};
