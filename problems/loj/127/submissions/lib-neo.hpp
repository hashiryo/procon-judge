#pragma once
// NeoLibrary の MaxFlow (neo/flow/MaxFlow.hpp、容量は i64) で解くもの。flow だけを呼ぶので、余りを s へ戻す段は回らない。
// hlpp_fifo との差は、辺を溜めてから CSR に組む形と、容量を i64 にした分の費用になる。
#include "neo/flow/MaxFlow.hpp"
struct Solver {
 MaxFlow<long long> g;
 int s, t;
 long long ans= 0;
 Solver(int n, int s, int t, const vector<array<int, 3>>& edges): g(n), s(s), t(t) {
  for(auto& e: edges) g.add_edge(e[0], e[1], e[2]);
 }
 void run() { ans= g.flow(s, t); }
 long long answer() const { return ans; }
};
