#pragma once
// 今の Library には増加路を 1 本ずつ足す口が無いので、二部マッチング (mylib/graph/BipartiteGraph.hpp の bipartite_matching)
// で、問題 0 から k - 1 までが全部解けるかを k について二分探索する。
#include "mylib/graph/BipartiteGraph.hpp"

struct Solver {
  int n;
  const vector<array<int, 2>> &q;
  Solver(int n, const vector<array<int, 2>> &q) : n(n), q(q) {}
  int run() {
    auto ok = [&](int k) {
      BipartiteGraph bg(k, n);
      for (int i = 0; i < k; ++i) {
        bg.add_edge(i, k + q[i][0]);
        if (q[i][1] != q[i][0]) bg.add_edge(i, k + q[i][1]);
      }
      return (int)bipartite_matching(bg).first.size() == k;
    };
    int lo = 0, hi = q.size();
    while (lo < hi) {
      const int mid = (lo + hi + 1) / 2;
      if (ok(mid)) lo = mid;
      else hi = mid - 1;
    }
    return lo;
  }
};
