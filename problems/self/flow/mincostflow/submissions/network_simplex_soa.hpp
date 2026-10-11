#pragma once
// 今の Library の NetworkSimplex と同じ手順 (block search) で、辺と頂点を配列ごとに分けて持つもの。中身は _shared/flow/mcf_ns.hpp にあり、yosupo-min-cost-b-flow の提出と共有する。
#include "_shared/flow/mcf_ns.hpp"
struct Solver {
 int n;
 const vector<i64>& b;
 const vector<array<i64, 5>>& e;
 mcf_ns::Result r;
 Solver(int n, const vector<i64>& b, const vector<array<i64, 5>>& edges): n(n), b(b), e(edges) {}
 void run() { r= mcf_ns::b_flow<1, 1>(n, b, e); }
 bool feasible() const { return r.ok; }
 __int128 cost() const { return r.cost; }
 const vector<i64>& potential() const { return r.pot; }
 const vector<i64>& flow() const { return r.flow; }
};
