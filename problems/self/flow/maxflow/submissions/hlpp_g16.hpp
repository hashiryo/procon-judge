#pragma once
// hlpp の global relabel の頻度を 8 分の 1 (仕事量が 16(6n + m) を超えるたび) にしたもの。
// 中身は _shared/flow/hlpp.hpp。
#include "_shared/flow/hlpp.hpp"
struct Solver {
 int n, s, t;
 const vector<array<int, 3>>& e;
 long long ans= 0;
 Solver(int n, int s, int t, const vector<array<int, 3>>& edges): n(n), s(s), t(t), e(edges) {}
 void run() { ans= mf_hlpp::max_flow<16>(n, s, t, e); }
 long long answer() const { return ans; }
};
