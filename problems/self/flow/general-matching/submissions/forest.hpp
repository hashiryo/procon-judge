#pragma once
// 空いている頂点すべてを根にした交互森を同時に育て、違う木がつながったら流してその 2 本をその回の残りで使わない
// (核は _shared/flow/gm_forest.hpp)。初期のマッチングは空。
#include "_shared/flow/gm_forest.hpp"
struct Solver {
 int n;
 const vector<array<int, 2>>& e;
 vector<int> mate;
 Solver(int n, const vector<array<int, 2>>& edges): n(n), e(edges) {}
 void run() { mate= gm_forest::general_matching<0, false>(n, e); }
 vector<int> answer() const { return gm_gabow::edge_ids(mate, e); }
};
