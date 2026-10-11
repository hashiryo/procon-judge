#pragma once
// 二部の二重被覆の最大マッチングを、もとのグラフの CSR を左右の隣接に使う push-relabel で求め、道と閉路に分けて組ませてから、交互森で仕上げる
// (核は _shared/flow/gm_dcpr.hpp)。
#include "_shared/flow/gm_dcpr.hpp"
struct Solver {
 int n;
 const vector<array<int, 2>>& e;
 vector<int> mate;
 Solver(int n, const vector<array<int, 2>>& edges): n(n), e(edges) {}
 void run() { mate= gm_dcpr::general_matching(n, e); }
 vector<int> answer() const { return gm_gabow::edge_ids(mate, e); }
};
