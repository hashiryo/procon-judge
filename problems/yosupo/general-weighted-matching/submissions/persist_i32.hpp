#pragma once
// persist_avx2 の値 (重み、双対、slack) を int32 にして、AVX2 で 8 列ずつ回す (核は _shared/flow/wm_persist_i32.hpp)。重みの
// 最大 W について 8W < 2^30 のときだけ使い、そうでなければ persist_avx2 の核で解く。
#include "_shared/flow/wm_persist_avx2.hpp"
#include "_shared/flow/wm_persist_i32.hpp"
struct Solver {
 int n;
 const vector<array<int, 3>>& e;
 vector<int> mate;
 Solver(int n, const vector<array<int, 3>>& edges): n(n), e(edges) {}
 template <class S> void solve_with() {
  S s(n);
  for(auto& [u, v, w]: e) s.add_edge(u, v, w);
  mate= s.solve();
 }
 void run() {
  long long wmax= 0;
  for(auto& x: e) wmax= max<long long>(wmax, x[2]);
  if(wmax * 8 < (1LL << 30)) solve_with<wm_persist_i32::Solver>();
  else solve_with<wm_persist_avx2::Solver>();
 }
 vector<int> answer() const {
  vector<int> ids;
  for(int i= 0; i < (int)e.size(); ++i)
   if(mate[e[i][0]] == e[i][1]) ids.push_back(i);
  return ids;
 }
};
