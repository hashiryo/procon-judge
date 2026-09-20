#pragma once
#include "pj.hpp"
#include "mylib/graph/Graph.hpp"
#include "mylib/graph/general_matching.hpp"

// 一般グラフの最大マッチング。二部であることを使わないので、二部専用の実装に
// 対する上界になる。
struct Solver {
  int l;
  Graph g;
  vector<int> matched;

  Solver(int l, int r, const vector<array<int, 2>> &edges) : l(l), g(l + r) {
    for (auto &e : edges) g.add_edge(e[0], e[1] + l);
  }

  void run() { matched = general_matching(g).first; }

  vector<array<int, 2>> answer() const {
    vector<array<int, 2>> ret;
    ret.reserve(matched.size());
    for (int i : matched) {
      auto [a, b] = g[i];
      ret.push_back({a, b - l});
    }
    return ret;
  }
};
