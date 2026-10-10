#pragma once
// よく知られた O(n^3) の主双対の花の方法を素直に書いたもの (核は _shared/flow/wm_std.hpp)。今の Library と同じ形。
#include "_shared/flow/wm_std.hpp"
struct Solver {
 int n;
 const vector<array<int, 3>>& e;
 vector<int> mate;
 Solver(int n, const vector<array<int, 3>>& edges): n(n), e(edges) {}
 void run() {
  wm_std::Solver s(n);
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
