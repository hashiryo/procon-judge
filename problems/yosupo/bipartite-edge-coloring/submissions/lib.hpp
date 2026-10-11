#pragma once
// 今の Library の bipartite_edge_coloring。頂点を次数の和が最大次数 D 以下になるようにまとめて D 正則に詰め、次数が偶数なら
// Euler 分割で半分に分け、奇数なら完全マッチング (Library の bipartite_matching の核) を 1 つ取る再帰。
#include "mylib/graph/BipartiteGraph.hpp"
#include "mylib/graph/bipartite_edge_coloring.hpp"
struct Solver {
 BipartiteGraph bg;
 vector<int> c;
 Solver(int l, int r, const vector<array<int, 2>>& edges): bg(l, r, edges.size()) {
  for(size_t i= 0; i < edges.size(); ++i) bg[i]= {edges[i][0], l + edges[i][1]};
 }
 void run() { c= bipartite_edge_coloring(bg); }
 const vector<int>& colors() const { return c; }
};
