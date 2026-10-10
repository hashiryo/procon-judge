#pragma once
// _shared/flow/proto_maxflow.hpp の試作の形 (容量は i64、flow のあとは正しい流れ) で解くもの。hlpp_fifo との差が、
// 余りを s へ戻す段と、辺を溜めてから CSR に組む形と、容量を i64 にした分の費用になる。
#include "_shared/flow/proto_maxflow.hpp"
struct Solver {
 proto::MaxFlow<long long> g;
 int s, t;
 long long ans= 0;
 Solver(int n, int s, int t, const vector<array<int, 3>>& edges): g(n), s(s), t(t) {
  for(auto& e: edges) g.add_edge(e[0], e[1], e[2]);
 }
 void run() { ans= g.flow(s, t); }
 long long answer() const { return ans; }
};
