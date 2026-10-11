#pragma once
// CSR に組んだ Dinic (multi-augment の DFS)。核は _dinic.hpp にある。
#include "_dinic.hpp"
struct Solver {
 int n, s, t;
 const vector<array<int, 4>>& e;
 long long ans= 0;
 Solver(int n, int s, int t, const vector<array<int, 4>>& edges): n(n), s(s), t(t), e(edges) {}
 void run() { ans= mf_dinic::max_flow<false>(n, s, t, e); }
 long long answer() const { return ans; }
};
