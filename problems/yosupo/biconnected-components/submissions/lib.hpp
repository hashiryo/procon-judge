#pragma once
// Library の block_cut_tree (mylib/graph/block_cut_tree.hpp)。Graph から無向の CSR を組み、行きがけ順と lowlink で block-cut
// tree を作る。頂点 [0, n) と成分 [n, K) を結ぶ木で、成分ごとの頂点の列は、成分の頂点の隣接として answer() で読み出す (raw の
// ときの lib.cpp は、ここまでを測っていた)。
#include <optional>
#include "pj.hpp"
#include "mylib/graph/Graph.hpp"
#include "mylib/graph/block_cut_tree.hpp"
struct Solver {
 int n;
 const vector<array<int, 2>>& es;
 optional<Graph> bct;
 Solver(int n, const vector<array<int, 2>>& es): n(n), es(es) {}
 void run() {
  Graph g(n, es.size());
  for(size_t i= 0; i < es.size(); ++i) g[i]= {es[i][0], es[i][1]};
  bct.emplace(block_cut_tree(g));
 }
 vector<vector<int>> answer() const {
  auto adj= bct->adjacency_vertex(0);
  vector<vector<int>> ret;
  for(int k= n; k < int(adj.size()); ++k) ret.emplace_back(adj[k].begin(), adj[k].end());
  return ret;
 }
};
