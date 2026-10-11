#pragma once
// persist_avx の花の代表の辺を花の行にだけ持ち、花を作るときの行の計算を AVX2 で回し、flower_from の表を親の花へのポインタに替える (核は _shared/flow/wm_persist_avx2.hpp)。
#include "_shared/flow/wm_persist_avx2.hpp"
struct Solver {
 int n;
 const vector<array<int, 3>>& e;
 vector<int> mate;
 Solver(int n, const vector<array<int, 3>>& edges): n(n), e(edges) {}
 void run() {
  wm_persist_avx2::Solver s(n);
  for(auto& [u, v, w]: e) s.add_edge(u, v, w);
  mate= s.solve();
 }
 vector<int> answer() const {
  vector<int> ids;
  for(int i= 0; i < (int)e.size(); ++i)
   if(mate[e[i][0]] == e[i][1]) ids.push_back(i);
  return ids;
 }
};
