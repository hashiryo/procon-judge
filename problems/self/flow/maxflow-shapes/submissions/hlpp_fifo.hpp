#pragma once
// HIPR 型の highest label の push-relabel で、同じ高さの活性な頂点を先に入れたものから取り出すもの。global relabel は HIPR の既定の頻度。核は _hlpp.hpp にある。
#include "_hlpp.hpp"
struct Solver {
 int n, s, t;
 const vector<array<int, 4>>& e;
 long long ans= 0;
 Solver(int n, int s, int t, const vector<array<int, 4>>& edges): n(n), s(s), t(t), e(edges) {}
 void run() { ans= mf_hlpp::max_flow<2, true>(n, s, t, e); }
 long long answer() const { return ans; }
};
