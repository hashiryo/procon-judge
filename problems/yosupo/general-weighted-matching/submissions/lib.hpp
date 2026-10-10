#pragma once
// 今の Library の WeightedMatching (頂点 2n の隣接行列を持つ O(n^3) の花の方法)。
#include "mylib/optimization/WeightedMatching.hpp"
struct Solver {
 int n;
 const vector<array<int, 3>>& e;
 WeightedMatching<long long> wm;
 Solver(int n, const vector<array<int, 3>>& edges): n(n), e(edges), wm(n) {
  for(auto& [u, v, w]: edges) wm.add_edge(u, v, w);
 }
 void run() { wm.build(); }
 vector<int> answer() const {
  vector<int> ids;
  for(int i= 0; i < (int)e.size(); ++i)
   if(wm.match(e[i][0]) == e[i][1]) ids.push_back(i);
  return ids;
 }
};
