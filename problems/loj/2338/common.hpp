#pragma once
// Kuhn 法の提出が共有するもの。問題を左、锦囊を右に置き、問題 0, 1, ... の順に増加路を探して、初めて見つからなかった
// ところで止める。使える锦囊が同じ問題は辺を 1 本にする。Match は _shared/kuhn/inc_matching.hpp の IncMatching の
// どれかで、提出が決めてから include する。
#include <vector>

struct Solver {
  int n;
  const vector<array<int, 2>> &q;
  Solver(int n, const vector<array<int, 2>> &q) : n(n), q(q) {}
  int run() {
    const int m = q.size();
    vector<array<int, 2>> e;
    for (int i = 0; i < m; ++i) {
      e.push_back({i, q[i][0]});
      if (q[i][1] != q[i][0]) e.push_back({i, q[i][1]});
    }
    Match mt(m, n, e);
    int k = 0;
    while (k < m && mt.augment(k)) ++k;
    return k;
  }
};
