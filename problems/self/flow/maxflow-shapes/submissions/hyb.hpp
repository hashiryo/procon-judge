#pragma once
// NeoLibrary の MaxFlow (容量は i64) に、HLPP が t に届かない余りの行き来で進まなくなったら増加路へ移る判定を足した試作
// (中身は _shared/flow/mf_hyb.hpp)。lib-neo と同じく flow だけを呼ぶ。
#include "_shared/flow/mf_hyb.hpp"
struct Solver {
 mf_hyb::MaxFlow<long long> g;
 int s, t;
 long long ans= 0;
 Solver(int n, int s, int t, const vector<array<int, 4>>& edges): g(n), s(s), t(t) {
  for(auto& e: edges) g.add_edge(e[0], e[1], e[2], e[3]);
 }
 void run() { ans= g.flow(s, t); }
 long long answer() const { return ans; }
};
