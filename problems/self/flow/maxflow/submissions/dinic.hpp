#pragma once
// Dinic。辺を CSR に並べ、DFS は 1 回の呼び出しで流せるだけ流す。中身は _shared/flow/dinic.hpp。
#include "_shared/flow/dinic.hpp"
struct Solver {
 int n, s, t;
 const vector<array<int, 3>>& e;
 long long ans= 0;
 Solver(int n, int s, int t, const vector<array<int, 3>>& edges): n(n), s(s), t(t), e(edges) {}
 void run() { ans= mf_dinic::max_flow<false>(n, s, t, e); }
 long long answer() const { return ans; }
};
