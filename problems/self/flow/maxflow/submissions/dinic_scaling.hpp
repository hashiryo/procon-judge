#pragma once
// 容量のスケーリングを入れた Dinic。中身は _shared/flow/dinic.hpp。
#include "_shared/flow/dinic.hpp"
struct Solver {
 int n, s, t;
 const vector<array<int, 3>>& e;
 long long ans= 0;
 Solver(int n, int s, int t, const vector<array<int, 3>>& edges): n(n), s(s), t(t), e(edges) {}
 void run() { ans= mf_dinic::max_flow<true>(n, s, t, e); }
 long long answer() const { return ans; }
};
