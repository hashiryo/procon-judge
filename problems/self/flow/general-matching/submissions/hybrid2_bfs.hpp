#pragma once
// hybrid2 の二重被覆の段で、頂点を BFS の順に番号を振り直してから解くもの。Karp-Sipser のあと交互森を 2 回まで回し、終わらなければ、そのマッチングから二重被覆の push-relabel を始めて丸め、交互森で
// 仕上げる (核は _shared/flow/gm_dcpr.hpp の hybrid)。
#include "_shared/flow/gm_dcpr.hpp"
struct Solver {
 int n;
 const vector<array<int, 2>>& e;
 vector<int> mate;
 Solver(int n, const vector<array<int, 2>>& edges): n(n), e(edges) {}
 void run() { mate= gm_dcpr::hybrid<true>(n, e, 2); }
 vector<int> answer() const { return gm_gabow::edge_ids(mate, e); }
};
