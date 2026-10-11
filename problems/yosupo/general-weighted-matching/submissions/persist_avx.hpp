#pragma once
// wm_persist の元の頂点についての走査 (行の走査、slack の付け直し、双対の動かし方の最小、きつい slack の検出) を AVX2 で回す (核は _shared/flow/wm_persist_avx.hpp)。
#include "_shared/flow/wm_persist_avx.hpp"
struct Solver {
 int n;
 const vector<array<int, 3>>& e;
 vector<int> mate;
 Solver(int n, const vector<array<int, 3>>& edges): n(n), e(edges) {}
 void run() {
  wm_persist_avx::Solver s(n);
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
