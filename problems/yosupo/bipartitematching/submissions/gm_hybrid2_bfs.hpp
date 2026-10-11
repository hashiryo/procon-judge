#pragma once
// gm_hybrid2 の二重被覆の段で、頂点を BFS の順に番号を振り直してから解くもの。一般グラフの最大マッチングで、Karp-Sipser のあと交互森を 2 回まで回し、終わらなければ、そのマッチングから二部の二重被覆の
// push-relabel を始めて丸め、交互森で仕上げるもの (核は _shared/flow/gm_dcpr.hpp の hybrid)。二部であることを使わない。
// 左 a を a、右 b を l + b とした一般グラフに渡す。
#include "_shared/flow/gm_dcpr.hpp"
struct Solver {
 int l;
 vector<array<int, 2>> g;
 vector<int> mate;
 Solver(int l, int r, const vector<array<int, 2>>& edges): l(l), g(edges.size()) {
  for(size_t i= 0; i < edges.size(); ++i) g[i]= {edges[i][0], l + edges[i][1]};
  mate.resize(l + r);
 }
 void run() { mate= gm_dcpr::hybrid<true>((int)mate.size(), g, 2); }
 vector<array<int, 2>> answer() const {
  vector<array<int, 2>> ret;
  for(int a= 0; a < l; ++a)
   if(mate[a] >= 0) ret.push_back({a, mate[a] - l});
  return ret;
 }
};
