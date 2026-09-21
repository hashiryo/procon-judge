#pragma once
#include <tuple>
#include "common.hpp"
#include "mylib/graph/BipartiteGraph.hpp"

// 2 部グラフの最大マッチングを増加路で取る。この問題の素直な形。
struct Solver {
  vector<string> grid;
  i64 ans = 0;

  explicit Solver(const vector<string> &grid) : grid(grid) {}

  void run() {
    Chocolate c = parse(grid);
    BipartiteGraph bg(c.cells, c.cells);
    for (auto &e : c.edges) bg.add_edge(e[0], e[1]);
    for (auto &[l, r] : bg) r += c.cells;
    auto res = bipartite_matching(bg);
    ans = happiness(c, (int)std::get<0>(res).size());
  }

  i64 answer() const { return ans; }
};
