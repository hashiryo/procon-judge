#pragma once
#include "pj.hpp"
#include "mylib/graph/Graph.hpp"
#include "mylib/graph/BipartiteGraph.hpp"
#include "mylib/graph/DulmageMendelsohn.hpp"

// 二部グラフに塗り分けてから Dulmage-Mendelsohn 分解で最小頂点被覆を取る。
// この問題のグラフは文字が隣り合う組だけを辺にするので二部になる。
struct Solver {
  int n;
  vector<array<int, 2>> edges;
  i64 ans = 0;

  Solver(int n, const vector<array<int, 2>> &edges) : n(n), edges(edges) {}

  void run() {
    Graph g(n);
    for (auto &e : edges) g.add_edge(e[0], e[1]);
    auto [bg, _, __] = graph_to_bipartite(g);
    DulmageMendelsohn dm(bg);
    ans = (i64)dm.min_vertex_cover().size();
  }

  i64 answer() const { return ans; }
};
