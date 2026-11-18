#pragma once
// Kuhn 法の提出が共有するもの。属性値を左 (10^4 個)、装備を右に置き、値 1, 2, ... の順に増加路を探して、初めて見つから
// なかったところで止める。属性値が同じ装備は辺を 1 本にする。Match は _shared/kuhn/inc_matching.hpp の IncMatching の
// どれかで、提出が決めてから include する。
#include <vector>

struct Solver {
  int n;
  const vector<array<int, 2>> &items;
  Solver(int n, const vector<array<int, 2>> &items) : n(n), items(items) {}
  int run() {
    constexpr int V = 10000;
    vector<array<int, 2>> e;
    e.reserve(2 * (size_t)n);
    for (int i = 0; i < n; ++i) {
      e.push_back({items[i][0] - 1, i});
      if (items[i][1] != items[i][0]) e.push_back({items[i][1] - 1, i});
    }
    Match m(V, n, e);
    int k = 0;
    while (k < V && m.augment(k)) ++k;
    return k;
  }
};
