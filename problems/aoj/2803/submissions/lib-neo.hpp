#pragma once
// NeoLibrary の MaxFlow (neo/flow/MaxFlow.hpp) で書いたもの。解き方は common.hpp と同じで、最小カットに乗る辺を 1 本ずつ
// change_cap で上限まで広げて流し直し、増分を見てから元の容量に戻す。容量を下げる change_cap のあとも最大流のまま
// なので、戻したあとに流し直さなくてよい。
#include "neo/flow/MaxFlow.hpp"
struct Solver {
 static constexpr i64 INF= 512345;
 int k, n;
 const vector<array<i64, 3>>& edges;
 i64 ans= 0;
 Solver(int k, int n, const vector<array<i64, 3>>& edges): k(k), n(n), edges(edges) {}
 void run() {
  MaxFlow<i64> g(n + k + 1);
  const int src= g.add_vertex();
  for(int j= 1; j <= k; ++j) g.add_edge(src, j, INF);
  vector<int> id(edges.size());
  for(size_t i= 0; i < edges.size(); ++i) id[i]= g.add_edge((int)edges[i][0], (int)edges[i][1], edges[i][2], edges[i][2]);
  ans= g.flow(src, 0);
  const auto cut= g.min_cut(src);
  i64 add= 0;
  for(size_t i= 0; i < edges.size(); ++i)
   if(cut[edges[i][0]] != cut[edges[i][1]]) {
    g.change_cap(id[i], INF, INF, src, 0);
    add= std::max(add, g.flow(src, 0));
    g.change_cap(id[i], edges[i][2], edges[i][2], src, 0);
   }
  ans+= add;
 }
 i64 answer() const { return ans; }
 bool unbounded() const { return ans >= INF; }
};
