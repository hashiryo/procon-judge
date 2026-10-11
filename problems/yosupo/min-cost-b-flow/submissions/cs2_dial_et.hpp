#pragma once
// cs2_dial に、早い打ち切り (ε が (n + 1) / 100 以下の段のあとで、元の費用の label-correcting が収まれば終える) を足したもの。中身は _shared/flow/mcf_cs2.hpp にあり、self-flow-mincostflow の提出と共有する。
#include "_shared/flow/mcf_cs2.hpp"
struct Solver {
 int n;
 const vector<i64>& b;
 const vector<array<i64, 5>>& e;
 mcf_cs2::Result r;
 Solver(int n, const vector<i64>& b, const vector<array<i64, 5>>& edges): n(n), b(b), e(edges) {}
 void run() { r= mcf_cs2::b_flow<16, 0, 1, 1, 1, 100>(n, b, e); }
 bool feasible() const { return r.ok; }
 __int128 cost() const { return r.cost; }
 const vector<i64>& potential() const { return r.pot; }
 const vector<i64>& flow() const { return r.flow; }
};
