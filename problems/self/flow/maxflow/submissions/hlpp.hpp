#pragma once
// highest label の push-relabel。global relabel は HIPR の既定の頻度 (仕事量が 2(6n + m) を超えるたび)。
// 中身は _shared/flow/hlpp.hpp。
#include "_shared/flow/hlpp.hpp"
struct Solver {
 int n, s, t;
 const vector<array<int, 3>>& e;
 long long ans= 0;
 Solver(int n, int s, int t, const vector<array<int, 3>>& edges): n(n), s(s), t(t), e(edges) {}
 void run() { ans= mf_hlpp::max_flow<2>(n, s, t, e); }
 long long answer() const { return ans; }
};
