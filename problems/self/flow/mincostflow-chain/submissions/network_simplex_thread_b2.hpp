#pragma once
// LEMON 型の木 (thread と部分木の大きさ) を持つネットワーク単体法で、block の大きさを 2 倍にしたもの。self-flow-mincostflow の
// network_simplex_thread_b2 と同じ手順で、核は _ns2.hpp (_shared/flow/mcf_ns2.hpp の写し) にある。
#include "_ns2.hpp"
struct Solver {
 int n;
 const vector<i64>& b;
 const vector<array<i64, 5>>& e;
 mcf_ns2::Result r;
 Solver(int n, const vector<i64>& b, const vector<array<i64, 5>>& edges): n(n), b(b), e(edges) {}
 void run() { r= mcf_ns2::b_flow<2, 1>(n, b, e); }
 bool feasible() const { return r.ok; }
 __int128 cost() const { return r.cost; }
 const vector<i64>& potential() const { return r.pot; }
 const vector<i64>& flow() const { return r.flow; }
};
