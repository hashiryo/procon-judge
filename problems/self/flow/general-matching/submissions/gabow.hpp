#pragma once
// 今の Library の general_matching と同じ探索を CSR と反復で書き直したもの (核は _shared/flow/gm_gabow.hpp)。初期のマッチングは空 (今の Library と同じ)。
#include "_shared/flow/gm_gabow.hpp"
struct Solver {
 int n;
 const vector<array<int, 2>>& e;
 vector<int> mate;
 Solver(int n, const vector<array<int, 2>>& edges): n(n), e(edges) {}
 void run() { mate= gm_gabow::general_matching<0>(n, e); }
 vector<int> answer() const { return gm_gabow::edge_ids(mate, e); }
};
