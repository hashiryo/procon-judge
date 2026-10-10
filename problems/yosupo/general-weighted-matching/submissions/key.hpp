#pragma once
// wm_std の元の頂点の slack を、その段で動かした双対の和を足した値で持ち、比べるたびに行列の列を引かない (核は _shared/flow/wm_key.hpp)。
#include "_shared/flow/wm_key.hpp"
struct Solver {
 int n;
 const vector<array<int, 3>>& e;
 vector<int> mate;
 Solver(int n, const vector<array<int, 3>>& edges): n(n), e(edges) {}
 void run() {
  wm_key::Solver s(n);
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
