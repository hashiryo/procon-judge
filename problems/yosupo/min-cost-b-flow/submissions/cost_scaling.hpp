#pragma once
// cost scaling (Goldberg と Tarjan の push-relabel 版)。中身は _shared/flow/mcf_cost_scaling.hpp にあり、self-flow-mincostflow
// の提出と共有する。
#include "_shared/flow/mcf_cost_scaling.hpp"
struct Solver {
 int n;
 const vector<i64>& b;
 const vector<array<i64, 5>>& e;
 mcf_cost_scaling::Result r;
 Solver(int n, const vector<i64>& b, const vector<array<i64, 5>>& edges): n(n), b(b), e(edges) {}
 void run() { r= mcf_cost_scaling::b_flow(n, b, e); }
 bool feasible() const { return r.ok; }
 __int128 cost() const { return r.cost; }
 const vector<i64>& potential() const { return r.pot; }
 const vector<i64>& flow() const { return r.flow; }
};
