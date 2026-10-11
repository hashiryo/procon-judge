#pragma once
// cs3_dial_et の流し方を partial augment (長さ 4 まで) にし、price update の間隔を relabel 2n 回ごとにしたもの。self-flow-mincostflow の cs3_pa4_et と同じ手順で、核は _cs3.hpp (_shared/flow/mcf_cs3.hpp の写し) にある。
#include "_cs3.hpp"
struct Solver {
 int n;
 const vector<i64>& b;
 const vector<array<i64, 5>>& e;
 mcf_cs3::Result r;
 Solver(int n, const vector<i64>& b, const vector<array<i64, 5>>& edges): n(n), b(b), e(edges) {}
 void run() { r= mcf_cs3::b_flow<16, 4, 2, 1, 1, 100>(n, b, e); }
 bool feasible() const { return r.ok; }
 __int128 cost() const { return r.cost; }
 const vector<i64>& potential() const { return r.pot; }
 const vector<i64>& flow() const { return r.flow; }
};
