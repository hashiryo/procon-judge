#pragma once
#include "pj.hpp"
#include "mylib/graph/BipartiteGraph.hpp"

// 二部グラフ専用の最大マッチング。Hopcroft-Karp。
struct Solver {
  int l;
  BipartiteGraph bg;
  vector<int> matched;

  Solver(int l, int r, const vector<array<int, 2>> &edges) : l(l), bg(l, r) {
    for (auto &e : edges) bg.add_edge(e[0], e[1] + l);
  }

  void run() { matched = bipartite_matching(bg).first; }

  vector<array<int, 2>> answer() const {
    vector<array<int, 2>> ret;
    ret.reserve(matched.size());
    for (int i : matched) {
      auto [a, b] = bg[i];
      ret.push_back({a, b - l});
    }
    return ret;
  }
};
