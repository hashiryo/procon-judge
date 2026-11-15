#pragma once
// NeoLibrary の max_weight_matching (neo/combinatorial_optimization/weighted_matching.hpp)。中身は persist_i32 と同じで、重みの
// 最大 W が 8W < 2^30 なら int32 の核、そうでなければ persist_avx2 の核で解く。
#include <tuple>
#include "neo/combinatorial_optimization/weighted_matching.hpp"
struct Solver {
 int n;
 const vector<array<int, 3>>& e;
 vector<tuple<int, int, int>> es;
 vector<int> mate;
 Solver(int n, const vector<array<int, 3>>& edges): n(n), e(edges) {
  es.reserve(edges.size());
  for(auto& [u, v, w]: edges) es.emplace_back(u, v, w);
 }
 void run() { mate= max_weight_matching(n, es).mate; }
 vector<int> answer() const {
  vector<int> ids;
  for(int i= 0; i < (int)e.size(); ++i)
   if(mate[e[i][0]] == e[i][1]) ids.push_back(i);
  return ids;
 }
};
