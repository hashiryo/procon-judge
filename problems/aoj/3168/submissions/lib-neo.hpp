#pragma once
// NeoLibrary の BipartiteMatching (neo/graph/BipartiteMatching.hpp) で書いたもの。グラフを bipartite_coloring で塗り分けて 2 つ目の入口に渡し、
// 最小点被覆の大きさは最大マッチングの大きさ (König の定理) で答える。
#include "neo/graph/BipartiteMatching.hpp"
struct Solver {
 int n;
 const vector<array<int, 2>>& edges;
 i64 ans= 0;
 Solver(int n, const vector<array<int, 2>>& edges): n(n), edges(edges) {}
 void run() { ans= BipartiteMatching(bipartite_coloring(n, edges), edges).size(); }
 i64 answer() const { return ans; }
};
