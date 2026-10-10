#pragma once
// hlpp で、同じ高さの活性な頂点を先に入れたものから取り出すもの (hlpp は後に入れたものから)。global relabel の頻度は
// hlpp と同じ。中身は _shared/flow/hlpp.hpp。
#include "_shared/flow/hlpp.hpp"
struct Solver {
 int n, s, t;
 const vector<array<int, 3>>& e;
 long long ans= 0;
 Solver(int n, int s, int t, const vector<array<int, 3>>& edges): n(n), s(s), t(t), e(edges) {}
 void run() { ans= mf_hlpp::max_flow<2, true>(n, s, t, e); }
 long long answer() const { return ans; }
};
