#pragma once
// 一般グラフの最大マッチングで、交互森を同時に育てる版に Karp-Sipser の初期化を足したもの (核は _shared/flow/gm_forest.hpp)。二部であることを使わない。左 a を a、右 b を l + b とした
// 一般グラフに渡す。
#include "_shared/flow/gm_forest.hpp"
struct Solver {
 int l;
 vector<array<int, 2>> g;
 vector<int> mate;
 Solver(int l, int r, const vector<array<int, 2>>& edges): l(l), g(edges.size()) {
  for(size_t i= 0; i < edges.size(); ++i) g[i]= {edges[i][0], l + edges[i][1]};
  mate.resize(l + r);
 }
 void run() { mate= gm_forest::general_matching<2, false>((int)mate.size(), g); }
 vector<array<int, 2>> answer() const {
  vector<array<int, 2>> ret;
  for(int a= 0; a < l; ++a)
   if(mate[a] >= 0) ret.push_back({a, mate[a] - l});
  return ret;
 }
};
