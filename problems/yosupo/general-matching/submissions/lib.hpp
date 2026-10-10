#pragma once
// 今の Library の general_matching (Graph を組んでから呼ぶ)。参照実装と同じもの。
#include "mylib/graph/Graph.hpp"
#include "mylib/graph/general_matching.hpp"
struct Solver {
 Graph g;
 vector<int> ids;
 Solver(int n, const vector<array<int, 2>>& edges): g(n) {
  for(auto& e: edges) g.add_edge(e[0], e[1]);
 }
 void run() { ids= general_matching(g).first; }
 vector<int> answer() const { return ids; }
};
