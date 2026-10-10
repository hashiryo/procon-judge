#pragma once
// wm_key の偶の頂点の行の走査を AVX2 で 4 列ずつ回し、花の中の頂点は花ごとに代表の辺で見る (核は _shared/flow/wm_avx.hpp)。
#include "_shared/flow/wm_avx.hpp"
struct Solver {
 int n;
 const vector<array<int, 3>>& e;
 vector<int> mate;
 Solver(int n, const vector<array<int, 3>>& edges): n(n), e(edges) {}
 void run() {
  wm_avx::Solver s(n);
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
