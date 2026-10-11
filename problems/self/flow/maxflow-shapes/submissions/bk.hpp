#pragma once
// Boykov-Kolmogorov (s と t から探索木を育て、孤児を付け直して使い続ける)。格子で速く、長い増加路で遅い。核は _bk.hpp にある。
#include "_bk.hpp"
struct Solver {
 int n, s, t;
 const vector<array<int, 4>>& e;
 long long ans= 0;
 Solver(int n, int s, int t, const vector<array<int, 4>>& edges): n(n), s(s), t(t), e(edges) {}
 void run() { ans= mf_bk::max_flow(n, s, t, e); }
 long long answer() const { return ans; }
};
