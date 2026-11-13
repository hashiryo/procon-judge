#pragma once
// NeoLibrary の PrimalDual (neo/flow/PrimalDual.hpp) で流すもの。中身は pd9 と同じ。
#include "neo/flow/PrimalDual.hpp"
struct Solver {
 PrimalDual<long long, long long> g;
 explicit Solver(int n): g(n) {}
 int add_vertex() { return g.add_vertex(); }
 int add_edge(int from, int to, i64 cap, i64 cost) { return g.add_edge(from, to, cap, cost); }
 vector<pair<i64, i64>> slope(int s, int t, i64 limit) { return g.slope(s, t, limit); }
 i64 flow(int i) const { return g.get_edge(i).flow; }
};
