#pragma once
// network_simplex_soa の block の大きさを 2 倍にしたもの。中身は _shared/flow/mcf_ns.hpp にあり、self-flow-mincostflow の提出と共有する。
#include "_shared/flow/mcf_ns.hpp"
struct Solver {
 int n;
 const vector<i64>& b;
 const vector<array<i64, 5>>& e;
 mcf_ns::Result r;
 Solver(int n, const vector<i64>& b, const vector<array<i64, 5>>& edges): n(n), b(b), e(edges) {}
 void run() { r= mcf_ns::b_flow<2, 1>(n, b, e); }
 bool feasible() const { return r.ok; }
 __int128 cost() const { return r.cost; }
 const vector<i64>& potential() const { return r.pot; }
 const vector<i64>& flow() const { return r.flow; }
};
