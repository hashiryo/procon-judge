#pragma once
// cost scaling に、global price update と push look-ahead を足したもの。中身は _shared/flow/mcf_cost_scaling.hpp にあり、yosupo-min-cost-b-flow の提出と共有する。
#include "_shared/flow/mcf_cost_scaling.hpp"
struct Solver {
 int n;
 const vector<i64>& b;
 const vector<array<i64, 5>>& e;
 mcf_cost_scaling::Result r;
 Solver(int n, const vector<i64>& b, const vector<array<i64, 5>>& edges): n(n), b(b), e(edges) {}
 void run() { r= mcf_cost_scaling::b_flow<16, true, true>(n, b, e); }
 bool feasible() const { return r.ok; }
 __int128 cost() const { return r.cost; }
 const vector<i64>& potential() const { return r.pot; }
 const vector<i64>& flow() const { return r.flow; }
};
