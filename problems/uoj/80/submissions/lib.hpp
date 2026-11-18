#pragma once
// 今の Library には完全でなくてよい二部グラフの割り当ての口が無いので、一般グラフの重み付きマッチング
// (mylib/optimization/WeightedMatching.hpp、完全でなくてよい重み最大) に、男 v を v、女 u を n_l + u として辺を張って解く。
#include "mylib/optimization/WeightedMatching.hpp"

struct Solver {
  int nl, nr;
  const vector<array<i64, 3>> &e;
  i64 total = 0;
  vector<int> mt;
  Solver(int nl, int nr, const vector<array<i64, 3>> &e) : nl(nl), nr(nr), e(e) {}
  void run() {
    WeightedMatching<i64> g(nl + nr);
    for (auto &[v, u, w] : e) g.add_edge(v, nl + u, w);
    g.build();
    total = 0;
    for (auto [a, b, w] : g.weight_matching()) total += w;
    mt.assign(nl, -1);
    for (int v = 0; v < nl; ++v)
      if (const int x = g.match(v); x >= nl) mt[v] = x - nl;
  }
  i64 sum() const { return total; }
  const vector<int> &mate() const { return mt; }
};
