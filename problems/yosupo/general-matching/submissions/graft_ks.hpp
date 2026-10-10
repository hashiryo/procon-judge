#pragma once
// 空いている頂点すべてを根にした交互森を最後まで保ち、流した 2 本の木だけを壊して、隣の生きている木に拾い直させる
// (核は _shared/flow/gm_graft.hpp)。初期のマッチングは Karp-Sipser。
#include "_shared/flow/gm_graft.hpp"
struct Solver {
 int n;
 const vector<array<int, 2>>& e;
 vector<int> mate;
 Solver(int n, const vector<array<int, 2>>& edges): n(n), e(edges) {}
 void run() { mate= gm_graft::general_matching<2>(n, e); }
 vector<int> answer() const { return gm_gabow::edge_ids(mate, e); }
};
