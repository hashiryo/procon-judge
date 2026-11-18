#pragma once
// 今の Library には増加路を 1 本ずつ足す口が無いので、二部マッチング (mylib/graph/BipartiteGraph.hpp の bipartite_matching)
// で、値 1 から k までが全部使えるかを k について二分探索する。k ごとに、値 1 から k を左、装備を右にしたグラフを組み
// 直して解く。
#include "mylib/graph/BipartiteGraph.hpp"

struct Solver {
  int n;
  const vector<array<int, 2>> &items;
  Solver(int n, const vector<array<int, 2>> &items) : n(n), items(items) {}
  int run() {
    auto ok = [&](int k) {
      BipartiteGraph bg(k, n);
      for (int i = 0; i < n; ++i) {
        const auto [a, b] = items[i];
        if (a <= k) bg.add_edge(a - 1, k + i);
        if (b != a && b <= k) bg.add_edge(b - 1, k + i);
      }
      return (int)bipartite_matching(bg).first.size() == k;
    };
    int lo = 0, hi = 10000;
    while (lo < hi) {
      const int mid = (lo + hi + 1) / 2;
      if (ok(mid)) lo = mid;
      else hi = mid - 1;
    }
    return lo;
  }
};
