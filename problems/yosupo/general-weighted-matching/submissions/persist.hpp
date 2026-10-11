#pragma once
// 増加路を流しても森を作り直さず、流した 2 本の木だけを壊して、ほかの木の印と slack を使い続ける。双対は遅延で持つ (核は _shared/flow/wm_persist.hpp)。
#include "_shared/flow/wm_persist.hpp"
struct Solver {
 int n;
 const vector<array<int, 3>>& e;
 vector<int> mate;
 Solver(int n, const vector<array<int, 3>>& edges): n(n), e(edges) {}
 void run() {
  wm_persist::Solver s(n);
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
